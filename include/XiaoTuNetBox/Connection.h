#ifndef XTNB_CONNECTION_H
#define XTNB_CONNECTION_H

#include <XiaoTuNetBox/DataQueue.hpp>
#include <XiaoTuNetBox/InBufObserver.h>
#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/EventHandler.h>
#include <XiaoTuNetBox/Types.h>

namespace xiaotu {
namespace net {
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
            void SendBytes(uint8_t const * buf, int num);
            void SendRawMsg(RawMsgPtr const & msg);
            void SendString(std::string const & msg);
            void OnReadEvent();
            void OnWriteEvent();
            void OnClosingEvent();
        private:
            int SendRawData(uint8_t const * buf, int num);

        public:
            InputBuffer & GetInputBuffer() { return mReadBuf; }
        private:
            std::string mInfoStr;
            PollEventHandlerPtr mEventHandler;
            InputBuffer mReadBuf;
            DataQueue<uint8_t> mWriteBuf;

        public:
            typedef std::function<void()> EventCallBk;

            void SetCloseCallBk(EventCallBk cb) { mCloseCallBk = std::move(cb); }
            void SetMsgCallBk(EventCallBk cb) { mMsgCallBk = std::move(cb); }

        private:
            EventCallBk mCloseCallBk;
            EventCallBk mMsgCallBk;
    };
}
}



#endif


