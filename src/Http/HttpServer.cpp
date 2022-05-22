#include <XiaoTuNetBox/Http/HttpServer.h>
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

    HttpServer::HttpServer(EventLoopPtr const & loop, int port, int max_conn)
        : TcpAppServer(loop, port, max_conn)
    {
        //mServer.SetTimeOut(2, 0, 3);
        mServer.SetNewConnCallBk(std::bind(&HttpServer::OnNewConnection, this, _1));
        mServer.SetCloseConnCallBk(std::bind(&HttpServer::OnCloseConnection, this, _1));
        mServer.SetMessageCallBk(std::bind(&HttpServer::OnMessage, this, _1, _2, _3));
    }

    // loop 线程
    void HttpServer::OnNewConnection(ConnectionPtr const & conn) {
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        std::cout << "新建连接 :" << conn->GetInfo() << std::endl;
        std::cout << "use count:" << conn.use_count() << std::endl;
        std::cout << "idx:     :" << conn->GetHandler()->GetLoopIdx() << std::endl;
        std::cout << "fd:      :" << conn->GetHandler()->GetFd() << std::endl;
        conn->GetHandler()->SetNonBlock(true);

        HttpSessionPtr ptr(new HttpSession());
        ptr->BuildWakeUpper(mServer.GetLoop(),
                std::bind(&HttpServer::HandleReponse, this, ConnectionWeakPtr(conn), HttpSessionWeakPtr(ptr)));

        conn->mUserObject = ptr;
        AddSession(ptr);
    }

    // loop 线程
    void HttpServer::HandleReponse(ConnectionWeakPtr const & conptr, HttpSessionWeakPtr const & sessptr)
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__  << std::endl;
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;
        HttpSessionPtr session = sessptr.lock();
        if (nullptr == session)
            return;

        HttpResponsePtr res = session->GetResponse();

        if (con->IsClosed()) {
            session->Reset();
            return;
        }

        std::vector<uint8_t> buf;
        res->ToUint8Vector(buf);
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        session->Reset();
    }

    // task 线程
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

        res->AppendContent(path, 0, s.st_size);
    }

    // task 线程
    void HttpServer::HandleRequest(ConnectionWeakPtr const & conptr, HttpSessionWeakPtr const & sessptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;

        HttpSessionPtr session = sessptr.lock();
        if (nullptr == session)
            return;

        HttpRequestPtr req = session->GetRequest();
        HttpResponsePtr res = session->GetResponse();

        res->SetClosing(!req->KeepAlive());
        res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);

        if (HttpRequest::eINVALID == req->GetMethod())
            res->SetStatusCode(HttpResponse::e400_BadRequest);

        if (HttpRequest::eGET == req->GetMethod())
            OnGetRequest(req, res);

        session->WakeUp();
    }

    // loop 线程
    void HttpServer::OnMessage(ConnectionPtr const & con,
                               uint8_t const * buf, ssize_t n)
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << "接收到了 " << n << "个字节:" << con->GetInfo() << std::endl;
        std::cout << "use count:" << con.use_count() << std::endl;
        std::cout << "idx:     :" << con->GetHandler()->GetLoopIdx() << std::endl;
        std::cout << "fd:      :" << con->GetHandler()->GetFd() << std::endl;

        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(con->mUserObject.lock());
        std::cout << ptr->GetStateStr() << std::endl;

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            begin = ptr->HandleRequest(begin, end);

            if (HttpSession::eResponsing == ptr->mState || HttpSession::eError == ptr->mState) {
                TaskPtr task(new Task(std::bind(&HttpServer::HandleRequest, this, ConnectionWeakPtr(con), HttpSessionWeakPtr(ptr))));
                AddTask(task);
            }

            if (nullptr == begin)
                break;
        }
    }

    // loop 线程
    void HttpServer::OnCloseConnection(ConnectionPtr const & conn) {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << "关闭连接 :" << conn->GetInfo() << std::endl;
        std::cout << "fd       :" << conn->GetHandler()->GetFd() << std::endl;
        std::cout << "Loop Idx :" << conn->GetHandler()->GetLoopIdx() << std::endl;

        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(conn->mUserObject.lock());
        ReleaseSession(ptr);
    }

}
}
