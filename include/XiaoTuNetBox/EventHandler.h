#ifndef XTNB_EVENTHANDLER_H
#define XTNB_EVENTHANDLER_H

#include <functional>
#include <poll.h>
#include <memory>
#include <thread>

namespace xiaotu {
namespace net {

    class EventHandler;
    typedef std::shared_ptr<EventHandler> EventHandlerPtr;
    typedef std::shared_ptr<EventHandler const> EventHandlerConstPtr;

    class EventLoop;
    class EventHandler {
        typedef std::shared_ptr<EventLoop> EventLoopPtr;
        public:
            EventHandler(int fd) : mFd(fd), mLoopIdx(-1) {}
            int GetFd() const { return mFd; }
            bool SetNonBlock(bool en);
        protected:
            int mFd;

        friend void ApplyHandlerOnLoop(EventHandlerPtr const & h, EventLoopPtr const & loop);
        friend void UnApplyHandlerOnLoop(EventHandlerPtr const & h, EventLoopPtr const & loop);
        public:
            int GetLoopIdx() const { return mLoopIdx; }
            EventLoopPtr GetLoop() { return mLoop; }
            std::thread::id GetLoopTid() const;
            void WakeUpLoop();
        protected:
            EventLoopPtr mLoop;
            int mLoopIdx;

        public:
            virtual void SetClosing(bool en) = 0;
            virtual void EnableRead(bool en) = 0;
            virtual void EnableWrite(bool en) = 0;

        public:
            typedef std::function<void()> EventCallBk;
            void SetReadCallBk(EventCallBk cb) { mReadCallBk = std::move(cb); }
            void SetWriteCallBk(EventCallBk cb) { mWriteCallBk = std::move(cb); }
            void SetClosingCallBk(EventCallBk cb) { mClosingCallBk = std::move(cb); }
        protected:
            EventCallBk mReadCallBk;
            EventCallBk mWriteCallBk;
            EventCallBk mClosingCallBk;
    };
}
}


#endif

