#ifndef XTNB_CONNECTION_H
#define XTNB_CONNECTION_H

#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/EventHandler.h>
#include <vector>

namespace xiaotu {
namespace net {
    typedef std::vector<char> RawMsg;
    typedef std::shared_ptr<RawMsg> RawMsgPtr;
    typedef std::shared_ptr<const RawMsg> RawMsgConstPtr;

    class Connection {
        public:
            Connection(int fd, IPv4Ptr const & peer);
            Connection(Connection const &) = delete;
            Connection & operator = (Connection const &) = delete;

            PollEventHandlerPtr & GetHandler() { return mEventHandler; }
            IPv4 const & GetPeerAddr() const { return *mPeerAddr; }

            void Close();
            void SendRawMsg(RawMsgPtr const & msg);
            void OnReadEvent();
            void OnWriteEvent();
            void OnClosingEvent();
        private:
            void SendRawData(char const * buf, int num);

        private:
            IPv4Ptr mPeerAddr;
            PollEventHandlerPtr mEventHandler;
            char mReadBuf[1024];
            std::vector<char> mWriteBuf;

        public:
            typedef std::function<void()> EventCallBk;
            typedef std::function<void(RawMsgPtr const &)> RawMsgCallBk;

            void SetCloseCallBk(EventCallBk cb) { mCloseCallBk = std::move(cb); }
            void SetRecvRawCallBk(RawMsgCallBk cb) { mRecvRawCallBk = std::move(cb); }


        private:
            EventCallBk mCloseCallBk;
            RawMsgCallBk mRecvRawCallBk;
    };

    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::shared_ptr<const Connection> ConnectionConstPtr;

}
}



#endif


