#include <XiaoTuNetBox/HttpServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <functional>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



namespace xiaotu {
namespace net {

    using namespace std::placeholders;

    HttpServer::HttpServer(PollLoopPtr const & loop, int port, int max_conn)
        : TcpAppServer(loop, port, max_conn)
    {
        mServer.SetTimeOut(10, 0, 5);
        mServer.SetNewConnCallBk(std::bind(&HttpServer::OnNewConnection, this, _1));
        mServer.SetCloseConnCallBk(std::bind(&HttpServer::OnCloseConnection, this, _1, _2));
        mServer.SetMessageCallBk(std::bind(&HttpServer::OnMessage, this, _1, _2));
    }

    SessionPtr HttpServer::OnNewConnection(ConnectionPtr const & conn) {
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        std::cout << "新建连接:" << conn->GetInfo() << std::endl;
        conn->GetHandler()->SetNonBlock(true);

        HttpSessionPtr ptr(new HttpSession(conn));
        ptr->mWakeUpper = std::make_shared<WakeUpper>();
        ptr->mWakeUpper->SetWakeUpCallBk(std::bind(&HttpServer::HandleReponse, this, conn, HttpSessionWeakPtr(ptr)));
        ApplyOnLoop(ptr->mWakeUpper, mServer.GetPollLoop());

        return AddSession(ptr);
    }

    void HttpServer::HandleReponse(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return;

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

    void HttpServer::OnGetRequest(HttpRequestPtr const & req, HttpResponsePtr const & res)
    {
        std::string urlpath = req->GetURLPath();
        if ("/" == urlpath)
            urlpath = "/index.html";

        std::string path = mWorkSpace + urlpath;
        std::cout << path << std::endl;

        struct stat s;
        if (!IsReadable(path) || (-1 == stat(path.c_str(), &s))) {
            res->SetStatusCode(HttpResponse::e404_NotFound);
            res->AppendContent("Error:404");
            return;
        }

        if (!S_ISREG(s.st_mode))
            return;

        res->SetStatusCode(HttpResponse::e200_OK);
        res->AppendContent(path, 0, s.st_size);
    }

    void HttpServer::HandleRequest(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return;

        HttpRequestPtr req = session->GetRequest();
        //req->PrintHeaders();
        std::string con_key("Connection");
        std::string con_header;
        bool has_con = req->GetHeader(con_key, con_header);

        if (has_con)
            ToLower(con_header);

        HttpResponsePtr res = session->GetResponse();
        bool close = !has_con || (con_header != "keep-alive");
        res->SetClosing(close);
        res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);

        if (HttpRequest::eGET == req->GetMethod())
            OnGetRequest(req, res);

        session->mWakeUpper->WakeUp(4096);
    }

    void HttpServer::OnMessage(ConnectionPtr const & con, SessionPtr const & session)
    {
        std::cout << "接收到了消息" << std::endl;
        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(session);
        std::cout << ptr->ToCString() << std::endl;
        std::cout << ptr->GetStateStr() << std::endl;

        assert(ptr->InRequestPhase());
        HttpRequestPtr request = ptr->HandleRequest(con);

        if (HttpSession::eResponsing == ptr->mState) {
            TaskPtr task(new Task(std::bind(&HttpServer::HandleRequest, this, con, HttpSessionWeakPtr(ptr))));
            AddTask(task);
        } else if (HttpSession::eError == ptr->mState) {
            con->SendString("HTTP/1.1 400 Bad Request\r\n\r\n");
            con->Close();
        }
    }

    void HttpServer::OnCloseConnection(ConnectionPtr const & conn, SessionPtr const & session) {
        std::cout << "关闭连接:" << conn->GetInfo() << std::endl;

        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(session);
        std::cout << ptr->ToCString() << std::endl;

        UnApplyOnLoop(ptr->mWakeUpper, mServer.GetPollLoop());
        
        ReleaseSession(session);
    }

}
}