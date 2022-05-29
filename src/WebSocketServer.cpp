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
#include <XiaoTuNetBox/Http/HttpModuleGet.h>
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
        HttpModulePtr checkURLModule = std::make_shared<HttpModuleCheckURL>(mWorkSpace);
        HttpModulePtr invalidRequestModule = std::make_shared<HttpModuleInvalidRequest>();
        //HttpModulePtr parseCookieModule = std::make_shared<HttpModuleParseCookie>();
        HttpModulePtr identifyModule = std::make_shared<HttpModuleBasicIdentify>();
        HttpModulePtr wsModule = std::make_shared<HttpModuleUpgradeWs>(this);
        HttpModulePtr getModule = std::make_shared<HttpModuleGet>();
        HttpModulePtr responseModule = std::make_shared<HttpModuleResponse>();

        invalidRequestModule->SetSuccessModule(identifyModule);
        invalidRequestModule->SetFailureModule(responseModule);

        //parseCookieModule->SetSuccessModule(identifyModule);

        identifyModule->SetSuccessModule(wsModule);
        identifyModule->SetFailureModule(responseModule);

        wsModule->SetFailureModule(checkURLModule);

        //checkURLModule->SetSuccessModule(parseCookieModule);
        checkURLModule->SetSuccessModule(getModule);
        checkURLModule->SetFailureModule(responseModule);

        getModule->SetFailureModule(responseModule);

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

    //! @brief 处理连接上的消息
    //!
    //! @param conn TCP 连接对象
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void WebSocketServer::OnMessage(ConnectionPtr const & conn, uint8_t const * buf, ssize_t n)
    {
        HandlerPtr session = std::static_pointer_cast<Handler>(conn->mUserObject.lock());

        if (typeid(HttpHandler).name() == session->ToCString())
            OnHttpMessage(conn, buf, n);
        else if (typeid(WebSocketHandler).name() == session->ToCString())
            OnWsMessage(conn, buf, n);

    }

    void WebSocketServer::OnCloseConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "关闭连接:" << conn->GetInfo();

        HandlerPtr session = std::static_pointer_cast<Handler>(conn->mUserObject.lock());
        ReleaseHandler(session);
    }


    void WebSocketServer::HandleWsReponse(ConnectionWeakPtr const & conptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;
        HandlerPtr session = std::static_pointer_cast<Handler>(con->mUserObject.lock());
        if (nullptr == session)
            return;

        assert(typeid(WebSocketHandler).name() == session->ToCString());
        WebSocketHandlerPtr ws = std::static_pointer_cast<WebSocketHandler>(session);

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
        WebSocketHandlerPtr session = std::static_pointer_cast<WebSocketHandler>(con->mUserObject.lock());
        assert(session->InOpenState());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            WebSocketMsgPtr msg = session->GetRcvdMsg();
            begin = msg->Parse(begin, end);

            if (WebSocketMsg::eNewMsg == msg->GetState()) {
                if (EWsOpcode::eWS_OPCODE_CLOSE == msg->GetHead()->opcode) {
                    LOG(INFO) << "关闭 WebSocket:" << *(msg->GetHead());
                    con->Close();
                } else {
                    if (mMsgCallBk)
                        mMsgCallBk(session, msg);

                    LOG(INFO) << session->GetStateStr() << " --> eOpen";
                    session->Reset();
                }
            } else if (WebSocketHandler::eError == session->mState) {
                con->Close();
            }

            if (nullptr == begin)
                break;
        }
    }

}
}


