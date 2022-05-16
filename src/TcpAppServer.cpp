/************************************************************************************
 * 
 * TcpAppServer - 应用层服务器
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/TcpAppServer.h>

#include <functional>

namespace xiaotu {
namespace net {

    TcpAppServer::TcpAppServer(EventLoopPtr const & loop, int port, int max_conn)
        : mServer(loop, port, max_conn)
    {
        mDestroing = false;
        mTaskThread = std::thread(std::bind(&TcpAppServer::FinishTasks, this));
    }
 
    TcpAppServer::~TcpAppServer()
    {
        std::unique_lock<std::mutex> lock(mFifoMutex);
        mDestroing = true;
        mFifoCV.notify_all();

        mTaskThread.join();
    }

    void TcpAppServer::AddTask(TaskPtr const & task)
    {
        std::unique_lock<std::mutex> lock(mFifoMutex);
        mTaskFifo.push_back(task);
        mFifoCV.notify_all();
    }

    void TcpAppServer::FinishTasks()
    {
        TaskPtr task = nullptr;

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
            if (nullptr != task) {
                task->Finish();
            }
        }
    }

    int TcpAppServer::GetFreeSessionIdx()
    {
        int idx = 0;
        if (mHoles.empty()) {
            idx = mSessions.size();
            mSessions.push_back(nullptr);
        } else {
            idx = mHoles.back();
            mHoles.pop_back();
            mSessions[idx] = nullptr;
        }
        return idx;
    }

    SessionPtr TcpAppServer::AddSession(SessionPtr const & ptr)
    {
        int idx = GetFreeSessionIdx();
        ptr->mIdx = idx;
        mSessions[idx] = ptr;
        return ptr;
    }

    void TcpAppServer::ReleaseSession(SessionPtr const & ptr)
    {
        assert(nullptr != ptr);
        assert(mSessions[ptr->mIdx] == ptr);

        ptr->ReleaseWakeUpper(mServer.GetLoop());
        ptr->mInBuf->Release();

        size_t & idx = ptr->mIdx;
        mSessions[idx].reset();
        mHoles.push_back(idx);
    }

}
}
