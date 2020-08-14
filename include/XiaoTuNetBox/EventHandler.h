#ifndef XTNB_EVENTHANDLER_H
#define XTNB_EVENTHANDLER_H

#include <functional>
#include <poll.h>
#include <memory>

namespace xiaotu {
namespace net {

    class PollLoop;
    typedef std::shared_ptr<PollLoop> PollLoopPtr;
    typedef std::shared_ptr<const PollLoop> PollLoopConstPtr;


    class PollEventHandler;
    typedef std::shared_ptr<PollEventHandler> PollEventHandlerPtr;
    typedef std::shared_ptr<const PollEventHandler> PollEventHandlerConstPtr;

    void ApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);
    void UnApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);

    class PollEventHandler : public std::enable_shared_from_this<PollEventHandler> {
        typedef std::shared_ptr<PollLoop> PollLoopPtr;

        public:
            PollEventHandler(int fd);
            PollEventHandler(PollEventHandler const &) = delete;
            PollEventHandler & operator = (PollEventHandler const &) = delete;


            void SetClosing(bool en);

            void EnableRead(bool en);
            void EnableWrite(bool en);
            void DisableAll() { mPollFd.events = 0; }
            bool IsReading() const { return (mPollFd.events & POLLIN); }
            bool IsWriting() const { return (mPollFd.events & POLLOUT); }
            struct pollfd const & GetPollFd() const { return mPollFd; }
            int GetFd() const { return mPollFd.fd; }

            int GetLoopIdx() const { return mLoopIdx; }
            pid_t GetLoopTid() const;
            void WakeUpLoop();

        friend void ApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);
        friend void UnApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);

        private:
            int mLoopIdx;
            PollLoopPtr mLoop;
            struct pollfd mPollFd;
            bool mIsClosing;

        public:
            void HandleEvents(struct pollfd const & pollFd);

            typedef std::function<void()> EventCallBk;
            void SetReadCallBk(EventCallBk cb) { mReadCallBk = std::move(cb); }
            void SetWriteCallBk(EventCallBk cb) { mWriteCallBk = std::move(cb); }
            void SetClosingCallBk(EventCallBk cb) { mClosingCallBk = std::move(cb); }
        private:
            EventCallBk mReadCallBk;
            EventCallBk mWriteCallBk;
            EventCallBk mClosingCallBk;
    };
}
}


#endif

