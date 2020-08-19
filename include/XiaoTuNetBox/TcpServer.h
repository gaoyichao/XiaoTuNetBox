#ifndef XTNB_TCPSERVER_H
#define XTNB_TCPSERVER_H

#include <XiaoTuNetBox/Acceptor.h>
#include <XiaoTuNetBox/ConnectionNode.h>
#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/EventHandler.h>
#include <XiaoTuNetBox/Timer.h>

#include <list>
#include <deque>
#include <memory>
#include <functional>
#include <unistd.h>

namespace xiaotu {
namespace net {
    class TcpServer {
        public:
            TcpServer(PollLoopPtr const & loop, int port, int max_conn);
            Acceptor & GetAcceptor() { return *mAcceptor; }

        private:
            void OnNewConnection(int fd, IPv4Ptr const &peer_addr);
            void OnCloseConnection(ConnectionNode * con);
            void OnNewRawMsg(ConnectionNode * con, RawMsgPtr const & msg);
            void OnTimeOut();


            PollLoopPtr mLoop;
            int mMaxConn;
            std::shared_ptr<Acceptor> mAcceptor;
            TimerPtr mTimer;
            /*
             * mTimeWheel - 时间轮盘,默认只有一格,调用SetTimeOut将扩展为n格
             */
            std::deque<ConnectionNode*> mTimeWheel;
            int mConnNum;

        public:
            typedef std::function<void(ConnectionPtr const & con)> ConnCallBk;
            typedef std::function<void(ConnectionPtr const & con, RawMsgPtr const &)> RawMsgCallBk;

            void SetNewConnCallBk(ConnCallBk cb) { mNewConnCallBk = std::move(cb); }
            void SetCloseConnCallBk(ConnCallBk cb) { mCloseConnCallBk = std::move(cb); }
            void SetNewRawMsgCallBk(RawMsgCallBk cb) { mNewRawMsgCallBk = std::move(cb); }
            /*
             * SetTimeOut - 超时关闭连接
             *
             * 时间轮盘刻度指每过一段时间检查一次时间轮盘，清空TimeWheel[0]
             * 各连接接收到数据后，就将自己从当前轮盘格中移除，放到TimeWheel[n-1]中
             *
             * @sec: 时间轮盘刻度(秒)
             * @nsec: 时间轮盘刻度(纳秒)
             * @n: 时间轮盘格数
             */
            void SetTimeOut(time_t sec, long nsec, int n); 


        private:
            ConnCallBk mNewConnCallBk;
            ConnCallBk mCloseConnCallBk;
            RawMsgCallBk mNewRawMsgCallBk;
    };
}
}

#endif

