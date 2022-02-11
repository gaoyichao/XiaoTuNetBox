/************************************************************************************
 * 
 * TcpAppServer - 应用层服务器
 * 
 ***********************************************************************************/
#ifndef XTNB_TCP_APP_SERVER_H
#define XTNB_TCP_APP_SERVER_H

#include <XiaoTuNetBox/TcpServer.h>
#include <XiaoTuNetBox/Task.h>

#include <memory>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace xiaotu {
namespace net {

    class TcpAppServer {
        public:
            TcpAppServer(PollLoopPtr const & loop, int port, int max_conn);
            ~TcpAppServer();
            TcpAppServer(TcpAppServer const &) = delete;
            TcpAppServer & operator = (TcpAppServer const &) = delete;

            std::string mWorkSpace;
        protected:
            virtual SessionPtr OnNewConnection(ConnectionPtr const & conn) = 0;
            virtual void OnCloseConnection(ConnectionPtr const & conn, SessionPtr const & session) = 0;
            virtual void OnMessage(ConnectionPtr const & con, SessionPtr const & session) = 0;

        protected:
            int GetFreeSessionIdx();
            SessionPtr AddSession(SessionPtr const & ptr);
            void ReleaseSession(SessionPtr const & ptr);

            TcpServer mServer;
            std::vector<SessionPtr> mSessions;
            std::vector<size_t> mHoles;

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

    typedef std::shared_ptr<TcpAppServer> TcpAppServerPtr;
    typedef std::weak_ptr<TcpAppServer> TcpAppServerWeakPtr;

}
}
 
#endif
