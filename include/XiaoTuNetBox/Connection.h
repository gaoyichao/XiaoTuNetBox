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

            PollEventHandlerPtr & GetHandler() { return mEventHandler; }

            void OnReadEvent();

            typedef std::function<void()> EventCallBk;
            void SetCloseCallBk(EventCallBk cb) { mCloseCallBk = std::move(cb); }

        private:
            IPv4Ptr mPeerAddr;
            PollEventHandlerPtr mEventHandler;
            char mReadBuf[1024];

            EventCallBk mCloseCallBk;
    };

    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::shared_ptr<const Connection> ConnectionConstPtr;

}
}



#endif


