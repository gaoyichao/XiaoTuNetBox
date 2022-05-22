/************************************************************************************
 * 
 * ThreadWorker - 线程工人 
 * 
 ***********************************************************************************/
#ifndef XTNB_THREAD_WORKER_H
#define XTNB_THREAD_WORKER_H

#include <XiaoTuNetBox/TcpServer.h>
#include <XiaoTuNetBox/Task.h>

#include <memory>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace xiaotu {
namespace net {

//! @defgroup ThreadUtils 各种线程工具
//! @{

    class ThreadWorker {
        public:
            ThreadWorker();
            ~ThreadWorker();

            ThreadWorker(ThreadWorker const &) = delete;
            ThreadWorker & operator = (ThreadWorker const &) = delete;

        public:
            void AddTask(TaskPtr const & task);
            void FinishTasks();
        private:
            std::mutex mFifoMutex;
            std::condition_variable mFifoCV;
            std::thread mTaskThread;
            bool mDestroing;
            std::deque<TaskPtr> mTaskFifo;

    };

    typedef std::shared_ptr<ThreadWorker> ThreadWorkerPtr;
    typedef std::weak_ptr<ThreadWorker> ThreadWorkerWeakPtr;

//! @}
}
}

 
#endif
