#ifndef XTNB_CONNECTION_H
#define XTNB_CONNECTION_H

#include <XiaoTuNetBox/DataQueue.hpp>
#include <XiaoTuNetBox/InBufObserver.h>
#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Types.h>
#include <XiaoTuNetBox/Event/EventLoop.h>
#include <XiaoTuNetBox/Event/EventHandler.h>

namespace xiaotu {
namespace net {
    class Connection;
    typedef std::weak_ptr<Connection> ConnectionWeakPtr;
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::shared_ptr<const Connection> ConnectionConstPtr;

    class Connection {
        public:
            Connection(int fd, std::string const & info, EventLoop const & loop);
            Connection(int fd, IPv4Ptr const & peer, EventLoop const & loop);
            Connection(Connection const &) = delete;
            Connection & operator = (Connection const &) = delete;
            ~Connection();
        private:
            void SetFd(int fd, EventLoop const & loop);

        public:
            EventHandlerPtr & GetHandler() { return mEventHandler; }
            std::string const & GetInfo() const { return mInfoStr; }

            bool IsClosed() const { return mIsClosed; }
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
            //! 用户定义的对象指针，其具体的数据类型和内存有用户自己管理
            ObjectWeakPtr mUserObject;
            static const size_t mReadBufSize;

        private:
            std::string mInfoStr;
            EventHandlerPtr mEventHandler;
            std::vector<uint8_t> mReadBuf;
            DataQueue<uint8_t> mWriteBuf;
            bool mIsClosed;

        public:
            typedef std::function<void()> EventCallBk;
            typedef std::function<void(uint8_t const * buf, ssize_t n)> BufCallBk;

            void SetCloseCallBk(EventCallBk cb) { mCloseCallBk = std::move(cb); }
            void SetMsgCallBk(BufCallBk cb) { mMsgCallBk = std::move(cb); }

        private:
            EventCallBk mCloseCallBk;
            BufCallBk mMsgCallBk;
    };
}
}



#endif


