/************************************************************************************
 * 
 * TcpAppServer - 应用层服务器
 * 
 ***********************************************************************************/
#ifndef XTNB_TCP_APP_SERVER_H
#define XTNB_TCP_APP_SERVER_H

#include <XiaoTuNetBox/TcpServer.h>
#include <XiaoTuNetBox/ThreadWorker.h>

#include <cassert>
#include <memory>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace xiaotu {
namespace net {

    class TcpAppServer {
        public:
            TcpAppServer(EventLoopPtr const & loop, int port,
                         int max_conn, std::string const & ws = ".");
            TcpAppServer(TcpAppServer const &) = delete;
            TcpAppServer & operator = (TcpAppServer const &) = delete;

            void SetWorker(ThreadWorkerPtr const & worker)
            {
                mWorker = worker;
            }

            void AddTask(TaskPtr const & task)
            {
                assert(mWorker);
                mWorker->AddTask(task);
            }

            HandlerPtr ReplaceHandler(HandlerPtr const & ori, HandlerPtr const & ptr);
            std::string mWorkSpace;
        protected:
            virtual void OnNewConnection(ConnectionPtr const & conn) = 0;
            virtual void OnCloseConnection(ConnectionPtr const & conn) = 0;
            virtual void OnMessage(ConnectionPtr const & con,
                                   uint8_t const * buf, ssize_t n) = 0;

        protected:
            int GetFreeHandlerIdx();
            HandlerPtr AddHandler(HandlerPtr const & ptr);
            void ReleaseHandler(HandlerPtr const & ptr);

            TcpServerPtr mServer;
            std::vector<HandlerPtr> mHandlers;
            std::vector<size_t> mHoles;

            ThreadWorkerPtr mWorker;
    };

    typedef std::shared_ptr<TcpAppServer> TcpAppServerPtr;
    typedef std::weak_ptr<TcpAppServer> TcpAppServerWeakPtr;

}
}
 
#endif
