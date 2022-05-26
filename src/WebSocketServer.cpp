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
                std::bind(&HttpServer::OnTaskFinished, ConnectionWeakPtr(conn)));

        conn->mUserObject = ptr;
        AddSession(ptr);
    }

    void WebSocketServer::HandleWsReponse(ConnectionWeakPtr const & conptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;
        SessionPtr session = std::static_pointer_cast<Session>(con->mUserObject.lock());
        if (nullptr == session)
            return;

        assert(typeid(WebSocketSession).name() == session->ToCString());
        WebSocketSessionPtr ws = std::static_pointer_cast<WebSocketSession>(session);

        if (WebSocketSession::eConnecting == ws->mState) {
            HttpResponsePtr res = ws->GetHandShakeResponse();

            std::vector<uint8_t> const & buf = res->GetContent();
            con->SendBytes(buf.data(), buf.size());
            if (res->CloseConnection())
                con->Close();

            LOG(INFO) << ws->GetStateStr() << " --> eOpen";
            ws->mState = WebSocketSession::eOpen;
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

    void WebSocketServer::OnCloseConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "关闭连接:" << conn->GetInfo();

        SessionPtr session = std::static_pointer_cast<Session>(conn->mUserObject.lock());
        ReleaseSession(session);
    }

    //! @brief 升级到 websocket 会话
    //!
    //! @param conptr TCP 连接对象
    //! @param weakptr Http 握手会话
    bool WebSocketServer::UpgradeSession(ConnectionWeakPtr const & conptr, HttpSessionWeakPtr const & weakptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return false;

        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpRequestPtr req = session->GetRequest();
        if (!WebSocketSession::CheckHandShake(req)) {
            session->WakeUp();
            return false;
        }

        WebSocketSessionPtr ptr(new WebSocketSession);
        ptr->AcceptHandShake(req);
        con->mUserObject = ptr;
        ReplaceSession(session, ptr);
        ptr->mWakeUpper->SetWakeUpCallBk(std::bind(&WebSocketServer::HandleWsReponse, this, ConnectionWeakPtr(con)));

        if (mNewSessionCallBk)
            mNewSessionCallBk(ptr);

        ptr->WakeUp();
        return true;
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
            HttpRequestPtr req = ptr->GetRequest();
            begin = req->Parse(begin, end);

            if (HttpRequest::eResponsing == req->GetState() || HttpRequest::eError == req->GetState()) {
                req->PrintHeaders();
                if (req->NeedUpgrade()) {
                    TaskPtr task(new Task(std::bind(&WebSocketServer::UpgradeSession, this, ConnectionWeakPtr(con), HttpSessionWeakPtr(ptr))));
                    ptr->mCurrTask = task;
                    AddTask(task);
                } else {
                    HttpServer::HandleRequest(con, mWorkSpace, mWorker);
                }
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


