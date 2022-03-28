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

    WebSocketServer::WebSocketServer(PollLoopPtr const & loop, int port, int max_conn)
        : TcpAppServer(loop, port, max_conn)
    {
        mServer->SetTimeOut(10, 0, 5);
        mServer->SetNewConnCallBk(std::bind(&WebSocketServer::OnNewConnection, this, _1));
        mServer->SetCloseConnCallBk(std::bind(&WebSocketServer::OnCloseConnection, this, _1));
        mServer->SetMessageCallBk(std::bind(&WebSocketServer::OnMessage, this, _1));
    }

    //! @brief 新建连接，与 PollLoop 在同一个进程中
    //!
    //! @param conn 连接对象
    void WebSocketServer::OnNewConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "新建连接:" << conn->GetInfo();
        conn->GetHandler()->SetNonBlock(true);

        HttpSessionPtr ptr(new HttpSession(conn));
        ptr->BuildWakeUpper(mServer->GetPollLoop(),
                            std::bind(&WebSocketServer::HandleReponse, this, conn));
        conn->mUserObject = ptr;
        AddSession(ptr);
    }

    //! @brief 处理Http的响应报文，与 PollLoop 在同一个进程中
    //!
    //! @param con 连接对象
    //! @param session Http会话对象
    void WebSocketServer::OnHttpResponse(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        HttpResponsePtr res = session->GetResponse();

        if (con->IsClosed())
            return;

        std::vector<uint8_t> buf;
        res->ToUint8Vector(buf);
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

        std::vector<uint8_t> buf;
        res->ToUint8Vector(buf);
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        LOG(INFO) << session->GetStateStr() << " --> eOpen";
        session->mState = WebSocketSession::eOpen;
    }

    //! @brief 处理响应报文，与 PollLoop 在同一个进程中, 由wakeupper触发
    void WebSocketServer::HandleReponse(ConnectionPtr const & con)
    {
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
        std::cout << path << std::endl;

        std::cout << "----------------------" << std::endl;
        //req->PrintHeaders();

        struct stat s;
        if (!IsReadable(path) || (-1 == stat(path.c_str(), &s))) {
            res->SetStatusCode(HttpResponse::e404_NotFound);
            res->AppendContent("Error:404");
            return;
        }

        if (!S_ISREG(s.st_mode))
            return;

        res->SetStatusCode(HttpResponse::e200_OK);

        //! @todo 检查文件类型, issue #3
        int idx = GetSuffix((uint8_t*)urlpath.data(), urlpath.size(), '.');
        std::string suffix = urlpath.substr(idx);
        if (".js" == suffix)
            res->SetHeader("Content-Type", "text/javascript");
        else if (".html" == suffix)
            res->SetHeader("Content-Type", "text/html");
        else if (".svg" == suffix)
            res->SetHeader("Content-Type", "image/svg+xml");

        res->AppendContent(path, 0, s.st_size);
    }

    //! @brief 处理接收到的消息, 运行在ThreadWorker线程中。
    //!
    //! @param con TCP 连接
    //! @param weakptr ws会话指针
    //! @param msg ws消息对象
    void WebSocketServer::HandleMessage(ConnectionPtr const & con, WebSocketSessionWeakPtr const & weakptr, WebSocketMsgPtr const & msg)
    {
        WebSocketSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return;

        if (mMsgCallBk)
            mMsgCallBk(session, msg);
    }

    void WebSocketServer::HandleRequest(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return;

        HttpRequestPtr req = session->GetRequest();
        HttpResponsePtr res = session->GetResponse();

        if (req->NeedUpgrade()) {
            WebSocketSessionPtr webptr = UpgradeSession(con, session);
            if (nullptr != webptr) {
                if (mNewSessionCallBk)
                    mNewSessionCallBk(webptr);
                webptr->WakeUp();
                return;
            }
        }

        res->SetClosing(!req->KeepAlive());
        res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);

        if (HttpRequest::eGET == req->GetMethod())
            OnGetRequest(req, res);

        session->WakeUp();
    }

    void WebSocketServer::OnCloseConnection(ConnectionPtr const & conn)
    {
        std::cout << "关闭连接:" << conn->GetInfo() << std::endl;

        SessionPtr session = std::static_pointer_cast<Session>(conn->mUserObject.lock());
        std::cout << session->ToCString() << std::endl;
        ReleaseSession(session);
    }

    //! @brief 升级到 websocket 会话
    //!
    //! @param con TCP 连接对象
    //! @param session Http 握手会话
    WebSocketSessionPtr WebSocketServer::UpgradeSession(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;

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
    //! @param session HttpSession 会话对象
    void WebSocketServer::OnHttpMessage(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        assert(session->InRequestPhase());
        HttpRequestPtr request = session->HandleRequest(con);

        if (HttpSession::eResponsing == session->GetState()) {
            TaskPtr task(new Task(std::bind(&WebSocketServer::HandleRequest, this, con, HttpSessionWeakPtr(session))));
            AddTask(task);
        } else if (HttpSession::eError == session->GetState()) {
            con->SendString("HTTP/1.1 400 Bad Request\r\n\r\n");
            con->Close();
        }
    }
    
    //! @brief 处理 websocket 消息
    //!
    //! @param con TCP 连接对象
    //! @param session WebSocketSession 会话对象
    void WebSocketServer::OnWsMessage(ConnectionPtr const & con, WebSocketSessionPtr const & session)
    {
        assert(session->InOpenState());
        WebSocketMsgPtr msg = session->HandleMsg(con);

        if (WebSocketSession::eNewMsg == session->mState) {

            if (EWsOpcode::eWS_OPCODE_CLOSE == msg->mHead.opcode) {
                LOG(INFO) << "关闭 WebSocket:" << msg->mHead;
                con->Close();
            } else {
                LOG(INFO) << session->GetStateStr() << " --> eOpen";
                session->mState = WebSocketSession::eOpen;
                TaskPtr task(new Task(std::bind(&WebSocketServer::HandleMessage, this, con, WebSocketSessionWeakPtr(session), msg)));
                AddTask(task);
            }
            
        } else if (WebSocketSession::eError == session->mState) {
            con->Close();
        }
    }

    void WebSocketServer::OnMessage(ConnectionPtr const & conn)
    {
        SessionPtr session = std::static_pointer_cast<Session>(conn->mUserObject.lock());
        //std::cout << "接收到了消息:" << session->ToCString() << std::endl;

        if (typeid(HttpSession).name() == session->ToCString())
            OnHttpMessage(conn, std::static_pointer_cast<HttpSession>(session));
        else if (typeid(WebSocketSession).name() == session->ToCString())
            OnWsMessage(conn, std::static_pointer_cast<WebSocketSession>(session));

    }

}
}


