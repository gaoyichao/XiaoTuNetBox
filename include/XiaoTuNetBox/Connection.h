#ifndef XTNB_CONNECTION_H
#define XTNB_CONNECTION_H

#include <XiaoTuNetBox/DataQueue.hpp>
#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/EventHandler.h>
#include <vector>

namespace xiaotu {
namespace net {
    typedef std::vector<char> RawMsg;
    typedef std::shared_ptr<RawMsg> RawMsgPtr;
    typedef std::shared_ptr<const RawMsg> RawMsgConstPtr;

    class Connection;
    typedef std::weak_ptr<Connection> ConnectionWeakPtr;
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::shared_ptr<const Connection> ConnectionConstPtr;

    class Connection {
        public:
            Connection(int fd, std::string const & info);
            Connection(int fd, IPv4Ptr const & peer);

            Connection(Connection const &) = delete;
            Connection & operator = (Connection const &) = delete;
        private:
            void SetFd(int fd);

        public:
            PollEventHandlerPtr & GetHandler() { return mEventHandler; }
            std::string const & GetInfo() const { return mInfoStr; }

            void Close();
            void SendBytes(char const * buf, int num);
            void SendRawMsg(RawMsgPtr const & msg);
            void OnReadEvent();
            void OnWriteEvent();
            void OnClosingEvent();
        private:
            int SendRawData(char const * buf, int num);

        private:
            std::string mInfoStr;
            PollEventHandlerPtr mEventHandler;
            char mReadBuf[1024];
            DataQueue<char> mWriteBuf;

        public:
            typedef std::function<void()> EventCallBk;
            typedef std::function<void(RawMsgPtr const &)> RawMsgCallBk;

            void SetCloseCallBk(EventCallBk cb) { mCloseCallBk = std::move(cb); }
            void SetRecvRawCallBk(RawMsgCallBk cb) { mRecvRawCallBk = std::move(cb); }


        private:
            EventCallBk mCloseCallBk;
            RawMsgCallBk mRecvRawCallBk;
    };
}
}



#endif


