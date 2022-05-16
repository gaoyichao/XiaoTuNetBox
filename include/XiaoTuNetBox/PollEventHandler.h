#ifndef XTNB_POLL_EVENT_HANDLER_H
#define XTNB_POLL_EVENT_HANDLER_H

#include <XiaoTuNetBox/EventHandler.h>

namespace xiaotu {
namespace net {


    class PollEventHandler;
    typedef std::shared_ptr<PollEventHandler> PollEventHandlerPtr;
    typedef std::shared_ptr<const PollEventHandler> PollEventHandlerConstPtr;

    class PollLoop;
    class PollEventHandler : public EventHandler {
        typedef std::shared_ptr<PollLoop> PollLoopPtr;

        public:
            PollEventHandler(int fd);
            PollEventHandler(PollEventHandler const &) = delete;
            PollEventHandler & operator = (PollEventHandler const &) = delete;

            void SetClosing(bool en);

            virtual void EnableRead(bool en) override;
            virtual void EnableWrite(bool en) override;

            void DisableAll() { mPollFd.events = 0; }
            bool IsReading() const { return (mPollFd.events & POLLIN); }
            bool IsWriting() const { return (mPollFd.events & POLLOUT); }
            struct pollfd const & GetPollFd() const { return mPollFd; }

            void HandleEvents(struct pollfd const & pollFd);
            PollLoopPtr GetPollLoop();
        private:
            struct pollfd mPollFd;
            bool mIsClosing;
            bool mIsClosed;
    };


}
}

#endif
