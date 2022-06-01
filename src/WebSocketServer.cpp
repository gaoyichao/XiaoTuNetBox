/************************************************************************************
 * 
 * WebSocketServer - WebSocket 的实现
 * 
 * https://datatracker.ietf.org/doc/html/rfc6455
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/WebSocketServer.h>
#include <XiaoTuNetBox/Http/HttpServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <XiaoTuNetBox/Http/HttpModuleCheckURL.h>
#include <XiaoTuNetBox/Http/HttpModuleIdentify.h>
#include <XiaoTuNetBox/Http/HttpModuleLogin.h>
#include <XiaoTuNetBox/Http/HttpModuleLoadFile.h>
#include <XiaoTuNetBox/HttpModuleUpgradeWs.h>


#include <cassert>
#include <functional>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glog/logging.h>

namespace xiaotu {
namespace net {
 
    using namespace std::placeholders;

    WebSocketServer::WebSocketServer(EventLoopPtr const & loop, int port,
                           int max_conn, std::string const & ws)
        : TcpAppServer(loop, port, max_conn, ws)
    {
        //mServer->SetTimeOut(10, 0, 5);
        mServer->SetNewConnCallBk(std::bind(&WebSocketServer::OnNewConnection, this, _1));
        mServer->SetCloseConnCallBk(std::bind(&WebSocketServer::OnCloseConnection, this, _1));
        mServer->SetMessageCallBk(std::bind(&WebSocketServer::OnMessage, this, _1, _2, _3));

        //! @todo 根据配置文件构建 Http 处理模块, mWorkSpace
        HttpModulePtr invalidRequestModule = std::make_shared<HttpModuleInvalidRequest>();
        HttpModulePtr parseCookieModule    = std::make_shared<HttpModuleParseCookie>();
        HttpModulePtr identifyModule       = std::make_shared<HttpModuleCookieIdentify>();
        HttpModuleLoginPtr loginModule     = std::make_shared<HttpModuleLogin>();
        HttpModulePtr checkURLModule       = std::make_shared<HttpModuleCheckURL>(mWorkSpace);
        HttpModuleCheckMethodPtr checkMethodModule = std::make_shared<HttpModuleCheckMethod>();
        HttpModulePtr unSupportModule      = std::make_shared<HttpModuleUnSupport>();
        HttpModulePtr loadFileModule       = std::make_shared<HttpModuleLoadFile>();
        HttpModulePtr responseModule       = std::make_shared<HttpModuleResponse>();
        HttpModulePtr wsModule = std::make_shared<HttpModuleUpgradeWs>(std::bind(
                    &WebSocketServer::OnWsHandler, this, _1, _2, _3));

        invalidRequestModule->SetSuccessModule(parseCookieModule);
        invalidRequestModule->SetFailureModule(responseModule);

        parseCookieModule->SetSuccessModule(identifyModule);

        identifyModule->SetSuccessModule(wsModule);
        identifyModule->SetFailureModule(loginModule);

        loginModule->mLoginHtml = "/login.html";
        loginModule->SetSuccessModule(checkURLModule);
        loginModule->SetFailureModule(responseModule);

        wsModule->SetFailureModule(checkURLModule);

        checkURLModule->SetSuccessModule(checkMethodModule);
        checkURLModule->SetFailureModule(responseModule);

        checkMethodModule->mOnGetModule = loadFileModule;
        checkMethodModule->mOnPostModule = loadFileModule;
        checkMethodModule->SetFailureModule(unSupportModule);

        loadFileModule->SetFailureModule(responseModule);
        unSupportModule->SetSuccessModule(responseModule);

        mFirstModule = invalidRequestModule;
    }

    //! @brief 新建连接，与 EventLoop 在同一个进程中
    //!
    //! @param conn 连接对象
    void WebSocketServer::OnNewConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "新建连接:" << conn->GetInfo();
        conn->GetHandler()->SetNonBlock(true);

        HttpHandlerPtr ptr(new HttpHandler(conn));
        ptr->BuildWakeUpper(mServer->GetLoop(),
                std::bind(&HttpServer::OnTaskFinished, ConnectionWeakPtr(conn)));

        conn->mUserObject = ptr;
        AddHandler(ptr);
    }

    //! @brief 从 Http 升级到 Websocket 协议
    //!
    //! @param con TCP连接
    //! @param h   原来的 Http 消息处理器
    //! @param wsh 新建的 Websocket 消息处理器
    void WebSocketServer::OnWsHandler(ConnectionPtr const & con,
                                      HttpHandlerPtr const & h,
                                      WebSocketHandlerPtr const & wsh)
    {
        ReplaceHandler(h, wsh);
        wsh->Reset();
        wsh->mWakeUpper->SetWakeUpCallBk(std::bind(&WebSocketServer::HandleWsReponse, con));
        if (mNewHandlerCallBk)
            mNewHandlerCallBk(wsh);
    }

    //! @brief 处理连接上的消息
    //!
    //! @param conn TCP 连接对象
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void WebSocketServer::OnMessage(ConnectionPtr const & conn, uint8_t const * buf, ssize_t n)
    {
        HandlerPtr h = std::static_pointer_cast<Handler>(conn->mUserObject.lock());

        if (typeid(HttpHandler).name() == h->ToCString())
            OnHttpMessage(conn, buf, n);
        else if (typeid(WebSocketHandler).name() == h->ToCString())
            OnWsMessage(conn, buf, n);

    }

    void WebSocketServer::OnCloseConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "关闭连接:" << conn->GetInfo();

        HandlerPtr h = std::static_pointer_cast<Handler>(conn->mUserObject.lock());
        if (mReleaseHandlerCallBk)
            mReleaseHandlerCallBk(std::static_pointer_cast<WebSocketHandler>(h));

        ReleaseHandler(h);
    }


    void WebSocketServer::HandleWsReponse(ConnectionPtr const & con)
    {
        HandlerPtr h = std::static_pointer_cast<Handler>(con->mUserObject.lock());
        if (nullptr == h)
            return;

        assert(typeid(WebSocketHandler).name() == h->ToCString());
        WebSocketHandlerPtr ws = std::static_pointer_cast<WebSocketHandler>(h);

        WebSocketMsgPtr sendmsg = ws->PopSendMsg();
        while (nullptr != sendmsg) {
            std::vector<uint8_t> const & buf = sendmsg->GetContent();
            con->SendBytes(buf.data(), buf.size());
            sendmsg = ws->PopSendMsg();
        }

    }


    //! @brief 解析消息，与 EventLoop 在同一个线程中，当成功解析处请求报文之后，
    //! 根据请求方法构建对应的处理任务
    //!
    //! @param con TCP 连接对象
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void WebSocketServer::OnHttpMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n)
    {
        LOG(INFO) << "接收到了消息";
        HttpHandlerPtr ptr = std::static_pointer_cast<HttpHandler>(con->mUserObject.lock());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            HttpRequestPtr req = ptr->GetRequest();
            begin = req->Parse(begin, end);

            if (HttpRequest::eResponsing == req->GetState() || HttpRequest::eError == req->GetState()) {
                mFirstModule->Handle(ptr);
            }

            if (nullptr == begin)
                break;
        }
    }
    
    //! @brief 处理 websocket 消息
    //!
    //! @param con TCP 连接对象
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void WebSocketServer::OnWsMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n)
    {
        WebSocketHandlerPtr h = std::static_pointer_cast<WebSocketHandler>(con->mUserObject.lock());
        assert(h->InOpenState());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            WebSocketMsgPtr msg = h->GetRcvdMsg();
            begin = msg->Parse(begin, end);

            if (WebSocketMsg::eNewMsg == msg->GetState()) {
                if (EWsOpcode::eWS_OPCODE_CLOSE == msg->GetHead()->opcode) {
                    LOG(INFO) << "关闭 WebSocket:" << *(msg->GetHead());
                    con->Close();
                } else {
                    if (mMsgCallBk)
                        mMsgCallBk(h, msg);

                    LOG(INFO) << h->GetStateStr() << " --> eOpen";
                    h->Reset();
                }
            } else if (WebSocketHandler::eError == h->mState) {
                con->Close();
            }

            if (nullptr == begin)
                break;
        }
    }

}
}


