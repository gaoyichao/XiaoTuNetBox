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
    const int HttpServer::mDefaultLoadSize = 8192;

    HttpServer::HttpServer(EventLoopPtr const & loop, int port, int max_conn)
        : TcpAppServer(loop, port, max_conn)
    {
        //mServer->SetTimeOut(2, 0, 3);
        mServer->SetNewConnCallBk(std::bind(&HttpServer::OnNewConnection, this, _1));
        mServer->SetCloseConnCallBk(std::bind(&HttpServer::OnCloseConnection, this, _1));
        mServer->SetMessageCallBk(std::bind(&HttpServer::OnMessage, this, _1, _2, _3));
    }
 
    //! @brief 新建连接，与 EventLoop 在同一个线程中
    //!
    //! @param conn 连接对象
    void HttpServer::OnNewConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "新建连接:" << conn->GetInfo();
        conn->GetHandler()->SetNonBlock(true);

        HttpSessionPtr ptr(new HttpSession());
        ptr->BuildWakeUpper(mServer->GetLoop(),
                std::bind(&HttpServer::OnTaskFinished, ConnectionWeakPtr(conn)));

        conn->mUserObject = ptr;
        AddSession(ptr);
    }

    //! @brief 解析消息，与 EventLoop 在同一个线程中，当成功解析处请求报文之后，
    //! 根据请求方法构建对应的处理任务
    //!
    //! @param con tcp连接
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void HttpServer::OnMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n)
    {
        LOG(INFO) << "接收到了消息";
        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(con->mUserObject.lock());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            begin = ptr->HandleRequest(begin, end);

            if (HttpSession::eResponsing == ptr->mState || HttpSession::eError == ptr->mState) {
                HandleRequest(con, mWorkSpace, mWorker);
            }

            if (nullptr == begin)
                break;
        }
    }

    // loop 线程
    void HttpServer::OnCloseConnection(ConnectionPtr const & conn) {
        LOG(INFO) << "关闭连接:" << conn->GetInfo();
        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(conn->mUserObject.lock());
        ReleaseSession(ptr);
    }

    /*************************************************************************************************/

    //! @brief 任务完成之后的回调函数，与 EventLoop 在同一个线程中
    void HttpServer::OnTaskFinished(ConnectionWeakPtr const & conptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;
        HttpSessionPtr session = std::static_pointer_cast<HttpSession>(con->mUserObject.lock());
        if (nullptr == session)
            return;

        if (con->IsClosed()) {
            session->Reset();
            return;
        }

        TaskPtr const & currTask = session->mCurrTask;
        if (currTask->success) {
            if (currTask->OnSuccess)
                currTask->Success();
            else
                HttpServer::OnTaskSuccessDefault(con, session);
        } else {
            if (currTask->OnFailure)
                currTask->Failure();
            else
                HttpServer::OnTaskFailureDefault(con, session);
        }
    }

    //! @brief 任务成功，一般处理方法，与 EventLoop 在同一个线程中
    bool HttpServer::OnTaskSuccessDefault(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        HttpResponsePtr res = session->GetResponse();

        std::vector<uint8_t> const & buf = res->GetContent();
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        session->Reset();
        return true;
    }

    //! @brief 任务失败，一般处理方法，与 EventLoop 在同一个线程中
    bool HttpServer::OnTaskFailureDefault(ConnectionPtr const & con, HttpSessionPtr const & session)
    {
        con->Close();
        session->Reset();
        return true;
    }

    bool HttpServer::OnTaskSuccessGet(ConnectionWeakPtr const & conptr, ThreadWorkerPtr const & worker)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return true;
        HttpSessionPtr session = std::static_pointer_cast<HttpSession>(con->mUserObject.lock());
        if (nullptr == session)
            return true;

        HttpResponsePtr res = session->GetResponse();

        int endidx = res->GetHeadEndIdx();
        int loadCount = res->GetLoadCount();
        size_t nleft = (0 == loadCount) ? 0 : res->GetDataSize();
        std::vector<uint8_t> const & buf = res->GetContent();

        if (loadCount <= 1) {
            con->SendBytes(buf.data(), buf.size());
        } else {
            con->SendBytes(buf.data() + endidx, buf.size() - endidx);
        }

        if (nleft > 0) {
            TaskPtr task = std::make_shared<Task>(std::bind(&HttpServer::HandleGetLoadContent, HttpSessionWeakPtr(session)));
            task->SetSuccessFunc(std::bind(&HttpServer::OnTaskSuccessGet, ConnectionWeakPtr(con), worker));
            worker->AddTask(task);
        } else {
            if (res->CloseConnection())
                con->Close();
            session->Reset();
        }
        return true;
    }

    //! @brief 与 EventLoop 在同一个线程中
    bool HttpServer::HandleRequest(ConnectionPtr const & con, std::string workspace, ThreadWorkerPtr const & worker)
    {
        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(con->mUserObject.lock());
        HttpRequestPtr req = ptr->GetRequest();
        HttpResponsePtr res = ptr->GetResponse();

        TaskPtr task = nullptr;

        req->mWorkSpace = workspace;
        res->SetClosing(!req->KeepAlive());

        switch (req->GetMethod()) {
            case HttpRequest::eINVALID:
                task = std::make_shared<Task>(std::bind(&HttpServer::HandleInvalidRequest, HttpSessionWeakPtr(ptr)));
                break;
            case HttpRequest::eHEAD:
                task = std::make_shared<Task>(std::bind(&HttpServer::HandleHeadRequest, HttpSessionWeakPtr(ptr)));
                break;
            case HttpRequest::eGET:
                task = std::make_shared<Task>(std::bind(&HttpServer::HandleGetRequest, HttpSessionWeakPtr(ptr)));
                task->SetSuccessFunc(std::bind(&HttpServer::OnTaskSuccessGet, ConnectionWeakPtr(con), worker));
                break;
            default:
                task = std::make_shared<Task>(std::bind(&HttpServer::HandleUnSupportRequest, HttpSessionWeakPtr(ptr)));
                break;
        }

        ptr->mCurrTask = task;
        worker->AddTask(task);

        return true;
    }

    bool HttpServer::HandleGetLoadContent(HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpResponsePtr res = session->GetResponse();
        res->LoadContent(mDefaultLoadSize);

        session->WakeUp();
        return true;
    }
    // task 线程
    bool HttpServer::HandleHeadRequest(HttpSessionWeakPtr const & weakptr) {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpRequestPtr req = session->GetRequest();
        HttpResponsePtr res = session->GetResponse();

        HttpServer::OnHeadRequest(req, res);

        session->WakeUp();
        return true;
    }

    // task 线程
    bool HttpServer::HandleGetRequest(HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpRequestPtr req = session->GetRequest();
        HttpResponsePtr res = session->GetResponse();

        HttpServer::OnHeadRequest(req, res);
        if (HttpResponse::e404_NotFound == res->GetStatusCode()) {
            res->AppendContent("Error:404");
        } else {
            res->LoadContent(mDefaultLoadSize);
        }

        session->WakeUp();
        return true;
    }

    // task 线程
    bool HttpServer::HandleInvalidRequest(HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpResponsePtr res = session->GetResponse();
        res->SetStatusCode(HttpResponse::e400_BadRequest);

        session->WakeUp();
        return true;
    }

    bool HttpServer::HandleUnSupportRequest (HttpSessionWeakPtr const & weakptr)
    {
        HttpSessionPtr session = weakptr.lock();
        if (nullptr == session)
            return false;

        HttpResponsePtr res = session->GetResponse();
        res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);

        session->WakeUp();
        return true;

    }

    //! @brief HEAD 请求处理函数
    void HttpServer::OnHeadRequest(HttpRequestPtr const & req, HttpResponsePtr const & res)
    {
        std::string urlpath = req->GetURLPath();
        if ("/" == urlpath)
            urlpath = "/index.html";

        std::string path = req->mWorkSpace + urlpath;
        LOG(INFO) << path;

        if (!IsReadable(path) || !res->SetFile(path)) {
            res->SetStatusCode(HttpResponse::e404_NotFound);
            std::string errstr("Error:404");
            res->LockHead(errstr.size());
            return;
        }

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
    }


}
}
