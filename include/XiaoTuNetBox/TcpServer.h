#ifndef XTNB_TCPSERVER_H
#define XTNB_TCPSERVER_H

#include <XiaoTuNetBox/Acceptor.h>
#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/EventHandler.h>

#include <list>
#include <memory>
#include <functional>
#include <unistd.h>

namespace xiaotu {
namespace net {
    class TcpServer {
        public:
            TcpServer(PollLoopPtr const & loop, int port, int max_conn);

            void OnNewConnection(int fd, IPv4Ptr const &peer_addr);
            void OnCloseConnection(ConnectionPtr const & con);
            void OnNewRawMsg(ConnectionPtr const & con, RawMsgPtr const & msg);

            Acceptor & GetAcceptor() { return *mAcceptor; }
        private:
            PollLoopPtr mLoop;
            int mMaxConn;
            std::shared_ptr<Acceptor> mAcceptor;
            std::list<ConnectionPtr> mConnList;

        public:
            typedef std::function<void(ConnectionPtr const & con)> ConnCallBk;
            typedef std::function<void(ConnectionPtr const & con, RawMsgPtr const &)> RawMsgCallBk;

            void SetNewConnCallBk(ConnCallBk cb) { mNewConnCallBk = std::move(cb); }
            void SetCloseConnCallBk(ConnCallBk cb) { mCloseConnCallBk = std::move(cb); }
            void SetNewRawMsgCallBk(RawMsgCallBk cb) { mNewRawMsgCallBk = std::move(cb); }

        private:
            ConnCallBk mNewConnCallBk;
            ConnCallBk mCloseConnCallBk;
            RawMsgCallBk mNewRawMsgCallBk;
    };
}
}

#endif

