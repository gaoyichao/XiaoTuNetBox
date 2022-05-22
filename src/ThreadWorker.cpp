/************************************************************************************
 * 
 * ThreadWorker - 线程工人 
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/ThreadWorker.h>
#include <XiaoTuNetBox/Task.h>

#include <memory>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace xiaotu {
namespace net {

    ThreadWorker::ThreadWorker()
    {
        mDestroing = false;
        mTaskThread = std::thread(std::bind(&ThreadWorker::FinishTasks, this));
    }


    ThreadWorker::~ThreadWorker()
    {
        std::unique_lock<std::mutex> lock(mFifoMutex);
        mDestroing = true;
        mFifoCV.notify_all();
        mTaskThread.join();
    }

    void ThreadWorker::FinishTasks()
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
            if (nullptr != task)
                task->Finish();
        }
    }

    void ThreadWorker::AddTask(TaskPtr const & task)
    {
        std::unique_lock<std::mutex> lock(mFifoMutex);
        mTaskFifo.push_back(task);
        mFifoCV.notify_all();
    }



}
}

