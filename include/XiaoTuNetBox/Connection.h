#ifndef XTNB_CONNECTION_H
#define XTNB_CONNECTION_H

#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/EventHandler.h>

namespace xiaotu {
namespace net {
    class Connection {
        public:
            Connection(int fd, IPv4Ptr const & peer);
            Connection(Connection const &) = delete;
            Connection & operator = (Connection const &) = delete;

            PollEventHandler & GetHandler() { return mEventHandler; }

            void OnReadEvent();

            typedef std::function<void(Connection const *)> EventCallBk;
            void SetCloseCallBk(EventCallBk cb) { mCloseCallBk = std::move(cb); }

        private:
            int mFd;
            IPv4Ptr mPeerAddr;
            PollEventHandler mEventHandler;
            char mReadBuf[1024];

            EventCallBk mCloseCallBk;
    };

    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::shared_ptr<const Connection> ConnectionConstPtr;

}
}



#endif


