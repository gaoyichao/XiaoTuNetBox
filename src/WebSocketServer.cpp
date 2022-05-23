/************************************************************************************
 * 
 * WebSocketServer - WebSocket 的实现
 * 
 * https://datatracker.ietf.org/doc/html/rfc6455
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/WebSocketServer.h>
#include <XiaoTuNetBox/Utils.h>

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

    WebSocketServer::WebSocketServer(EventLoopPtr const & loop, int port, int max_conn)
        : TcpAppServer(loop, port, max_conn)
    {
        //mServer->SetTimeOut(10, 0, 5);
        mServer->SetNewConnCallBk(std::bind(&WebSocketServer::OnNewConnection, this, _1));
        mServer->SetCloseConnCallBk(std::bind(&WebSocketServer::OnCloseConnection, this, _1));
        mServer->SetMessageCallBk(std::bind(&WebSocketServer::OnMessage, this, _1, _2, _3));
    }

    //! @brief 新建连接，与 EventLoop 在同一个进程中
    //!
    //! @param conn 连接对象
    void WebSocketServer::OnNewConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "新建连接:" << conn->GetInfo();
        conn->GetHandler()->SetNonBlock(true);

        HttpSessionPtr ptr(new HttpSession());
        ptr->BuildWakeUpper(mServer->GetLoop(),
                std::bind(&WebSocketServer::HandleReponse, this, ConnectionWeakPtr(conn)));

        conn->mUserObject = ptr;
        AddSession(ptr);
    }

    //! @brief 处理Http的响应报文，与 EventLoop 在同一个进程中
    //!
    //! @param con 连接对象
    //! @param session Http会话对象
    void WebSocketServer::OnHttpResponse(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        HttpResponsePtr res = session->GetResponse();

        if (con->IsClosed()) {
            session->Reset();
            return;
        }

        std::vector<uint8_t> const & buf = res->GetContent();
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        session->Reset();
    }

    //! @brief Websocket 握手响应，与 PollLoop 在同一个进程中
    //! 切换状态 eConnecting --> eOpen
    //!
    //! @param con TCP 连接对象
    //! @param session 会话对象
    void WebSocketServer::OnWsConResponse(ConnectionPtr const & con, WebSocketSessionPtr const & session)
    {
        HttpResponsePtr res = session->GetHandShakeResponse();

        if (con->IsClosed())
            return;

        std::vector<uint8_t> const & buf = res->GetContent();
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        LOG(INFO) << session->GetStateStr() << " --> eOpen";
        session->mState = WebSocketSession::eOpen;
    }

    //! @brief 处理响应报文，与 EventLoop 在同一个进程中, 由wakeupper触发
    void WebSocketServer::HandleReponse(ConnectionWeakPtr const & conptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;
        SessionPtr session = std::static_pointer_cast<Session>(con->mUserObject.lock());
        if (nullptr == session)
            return;

        if (typeid(HttpSession).name() == session->ToCString()) {
            OnHttpResponse(con, std::static_pointer_cast<HttpSession>(session));
            return;
        }

        assert (typeid(WebSocketSession).name() == session->ToCString());
        WebSocketSessionPtr ws = std::static_pointer_cast<WebSocketSession>(session);

        if (WebSocketSession::eConnecting == ws->mState) {
            OnWsConResponse(con, ws);
            return;
        }

        while (true) {
            RawMsgPtr sendmsg = ws->PopSendMsg();
            if (nullptr != sendmsg)
                con->SendBytes(sendmsg->data(), sendmsg->size());
            else
                break;
        }
    }

    void WebSocketServer::OnGetRequest(HttpRequestPtr const & req, HttpResponsePtr const & res)
    {
        std::string urlpath = req->GetURLPath();
        if ("/" == urlpath)
            urlpath = "/index.html";

        std::string path = mWorkSpace + urlpath;
        LOG(INFO) << path;

        if (!IsReadable(path)) {
            res->SetStatusCode(HttpResponse::e404_NotFound);
            std::string errstr("Error:404");
            res->LockHead(errstr.size());
            res->AppendContent("Error:404");
            return;
        }

        if (!res->SetFile(path))
            return;

        //! @todo 检查文件类型, issue #3
        int idx = GetSuffix((uint8_t*)urlpath.data(), urlpath.size(), '.');
        if (idx > 0) {
            std::string suffix = urlpath.substr(idx);
            if (".js" == suffix)
                res->SetHeader("Content-Type", "text/javascript");
            else if (".html" == suffix)
                res->SetHeader("Content-Type", "text/html");
            else if (".svg" == suffix)
                res->SetHeader("Content-Type", "image/svg+xml");
        }

        struct stat const & s = res->GetFileStat();
        res->SetStatusCode(HttpResponse::e200_OK);
        res->LockHead(s.st_size);
        res->LoadContent(res->GetFileStat().st_size);
    }

    //! @brief 处理接收到的消息, 运行在ThreadWorker线程中。
    //!
    //! @param weakptr ws会话指针
    //! @param msg ws消息对象
    bool WebSocketServer::HandleMessage(WebSocketSessionWeakPtr const & weakptr, WebSocketMsgPtr const & msg)
    {
        WebSocketSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        if (mMsgCallBk)
            mMsgCallBk(session, msg);

        return true;
    }

    bool WebSocketServer::HandleRequest(ConnectionWeakPtr const & conptr, HttpSessionWeakPtr const & weakptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return false;

        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpRequestPtr req = session->GetRequest();
        HttpResponsePtr res = session->GetResponse();

        res->SetClosing(!req->KeepAlive());
        res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);

        if (req->NeedUpgrade()) {
            WebSocketSessionPtr webptr = UpgradeSession(con, session);
            if (nullptr != webptr) {
                if (mNewSessionCallBk)
                    mNewSessionCallBk(webptr);
                webptr->WakeUp();
            } else {
                session->WakeUp();
            }
            return true;
        }

        if (HttpRequest::eGET == req->GetMethod())
            OnGetRequest(req, res);

        session->WakeUp();
        return true;
    }

    void WebSocketServer::OnCloseConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "关闭连接:" << conn->GetInfo();

        SessionPtr session = std::static_pointer_cast<Session>(conn->mUserObject.lock());
        ReleaseSession(session);
    }

    //! @brief 升级到 websocket 会话
    //!
    //! @param con TCP 连接对象
    //! @param session Http 握手会话
    WebSocketSessionPtr WebSocketServer::UpgradeSession(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        LOG(INFO) << "升级 websocket";

        HttpRequestPtr req = session->GetRequest();

        if (!WebSocketSession::CheckHandShake(req)) {
            return nullptr;
        }

        WebSocketSessionPtr ptr(new WebSocketSession);
        ptr->AcceptHandShake(req);
        con->mUserObject = ptr;
        ReplaceSession(session, ptr);

        return ptr;
    }

    //! @brief 处理 Http 请求
    //!
    //! @param con TCP 连接对象
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void WebSocketServer::OnHttpMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n)
    {
        LOG(INFO) << "接收到了 Http 消息";
        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(con->mUserObject.lock());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            begin = ptr->HandleRequest(begin, end);

            if (HttpSession::eResponsing == ptr->GetState() || HttpSession::eError == ptr->GetState()) {
                TaskPtr task(new Task(std::bind(&WebSocketServer::HandleRequest, this, ConnectionWeakPtr(con), HttpSessionWeakPtr(ptr))));
                AddTask(task);
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
        LOG(INFO) << "接收到了 ws 消息";
        WebSocketSessionPtr session = std::static_pointer_cast<WebSocketSession>(con->mUserObject.lock());
        assert(session->InOpenState());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            begin = session->HandleMsg(begin, end);

            if (WebSocketSession::eNewMsg == session->mState) {
                WebSocketMsgPtr msg = session->GetRcvdMsg();

                if (EWsOpcode::eWS_OPCODE_CLOSE == msg->mHead.opcode) {
                    LOG(INFO) << "关闭 WebSocket:" << msg->mHead;
                    con->Close();
                } else {
                    TaskPtr task(new Task(std::bind(&WebSocketServer::HandleMessage, this, WebSocketSessionWeakPtr(session), msg)));
                    AddTask(task);
                    LOG(INFO) << session->GetStateStr() << " --> eOpen";
                    session->mState = WebSocketSession::eOpen;
                }
            } else if (WebSocketSession::eError == session->mState) {
                con->Close();
            }

            if (nullptr == begin)
                break;
        }



    }

    //! @brief 处理连接上的消息
    //!
    //! @param conn TCP 连接对象
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void WebSocketServer::OnMessage(ConnectionPtr const & conn, uint8_t const * buf, ssize_t n)
    {
        SessionPtr session = std::static_pointer_cast<Session>(conn->mUserObject.lock());

        if (typeid(HttpSession).name() == session->ToCString())
            OnHttpMessage(conn, buf, n);
        else if (typeid(WebSocketSession).name() == session->ToCString())
            OnWsMessage(conn, buf, n);

    }

}
}


