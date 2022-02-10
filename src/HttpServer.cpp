#include <XiaoTuNetBox/HttpServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <functional>
#include <iostream>


namespace xiaotu {
namespace net {

    using namespace std::placeholders;

    HttpServer::HttpServer(PollLoopPtr const & loop, int port, int max_conn)
        : mServer(loop, port, max_conn)
    {
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        mServer.SetTimeOut(10, 0, 5);
        mServer.SetNewConnCallBk(std::bind(&HttpServer::OnNewConnection, this, _1));
        mServer.SetCloseConnCallBk(std::bind(&HttpServer::OnCloseConnection, this, _1, _2));
        mServer.SetMessageCallBk(std::bind(&HttpServer::OnMessage, this, _1, _2));

        mDestroing = false;
        mTaskThread = std::thread(std::bind(&HttpServer::FinishTasks, this));
    }

    HttpServer::~HttpServer()
    {
        std::unique_lock<std::mutex> lock(mFifoMutex);
        mDestroing = true;
        mFifoCV.notify_all();

        mTaskThread.join();
    }


    SessionPtr HttpServer::OnNewConnection(ConnectionPtr const & conn) {
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        std::cout << "新建连接:" << conn->GetInfo() << std::endl;
        conn->GetHandler()->SetNonBlock(true);

        int idx = 0;
        if (mHoles.empty()) {
            idx = mSessions.size();
            mSessions.push_back(nullptr);
        } else {
            idx = mHoles.back();
            mHoles.pop_back();
            mSessions[idx] = nullptr;
        }
        HttpSessionPtr ptr(new HttpSession(conn));
        ptr->mIdx = idx;
        ptr->mWakeUpper = std::make_shared<WakeUpper>();
        ptr->mWakeUpper->SetWakeUpCallBk(std::bind(&HttpServer::HandleReponse, this, conn, HttpSessionWeakPtr(ptr)));
        ApplyOnLoop(ptr->mWakeUpper, mServer.GetPollLoop());

        mSessions[idx] = std::move(ptr);
        return mSessions[idx];
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

        if (mRequestCB)
            mRequestCB(req, res);

        session->mWakeUpper->WakeUp(4096);
    }

    void HttpServer::FinishTasks()
    {
        HttpTaskPtr task = nullptr;

        while (true) {
            {
                std::unique_lock<std::mutex> lock(mFifoMutex);
                while (mTaskFifo.empty() && !mDestroing)
                    mFifoCV.wait(lock);
                if (mDestroing)
                    break;
                task = mTaskFifo.front();
                mTaskFifo.pop_front();
            }

            std::cout << __FUNCTION__ << std::endl;
            if (nullptr != task)
                task->Finish();
        }
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
            HttpTaskPtr task(new HttpTask);
            task->SetTaskFunc(std::bind(&HttpServer::HandleRequest, this, con, HttpSessionWeakPtr(ptr)));

            std::unique_lock<std::mutex> lock(mFifoMutex);
            mTaskFifo.push_back(task);
            mFifoCV.notify_all();
        } else if (HttpSession::eError == ptr->mState) {
            con->SendString("HTTP/1.1 400 Bad Request\r\n\r\n");
            con->Close();
        }
    }

    void HttpServer::OnCloseConnection(ConnectionPtr const & conn, SessionPtr const & session) {
        std::cout << "关闭连接:" << conn->GetInfo() << std::endl;

        HttpSessionPtr ptr = std::static_pointer_cast<HttpSession>(session);
        std::cout << ptr->ToCString() << std::endl;

        std::cout << "连接[" << conn->GetInfo() << "]关闭,释放 buffer" << std::endl;
        ptr->mInBuf->Release();
        UnApplyOnLoop(ptr->mWakeUpper, mServer.GetPollLoop());
        
        size_t & idx = ptr->mIdx;
        mSessions[idx].reset();
        mHoles.push_back(idx);

        std::cout << "numholes:" << conn->GetInputBuffer().NumHoles() << std::endl;
        std::cout << "numobses:" << conn->GetInputBuffer().NumObservers() << std::endl;
        std::cout << "obsize:" << conn->GetInputBuffer().ObserverSize() << std::endl;
    }



}
}
