#ifndef XTNB_EPOLL_EVENT_HANDLER_H
#define XTNB_EPOLL_EVENT_HANDLER_H

#include <XiaoTuNetBox/EventHandler.h>

#include <sys/epoll.h>

namespace xiaotu {
namespace net {

    class EPollEventHandler;
    typedef std::shared_ptr<EPollEventHandler> EPollEventHandlerPtr;
    typedef std::shared_ptr<const EPollEventHandler> EPollEventHandlerConstPtr;


    class EPollLoop;
    class EPollEventHandler : public EventHandler {
        typedef std::shared_ptr<EPollLoop> EPollLoopPtr;

        public:
            EPollEventHandler(int fd);
            EPollEventHandler(EPollEventHandler const &) = delete;
            EPollEventHandler & operator = (EPollEventHandler const &) = delete;

            //void SetClosing(bool en);
            void UseEdgeTrigger(bool en);

            virtual void EnableRead(bool en) override;
            virtual void EnableWrite(bool en) override;

            //void DisableAll() { mPollFd.events = 0; }
            //bool IsReading() const { return (mPollFd.events & POLLIN); }
            //bool IsWriting() const { return (mPollFd.events & POLLOUT); }
            //struct pollfd const & GetPollFd() const { return mPollFd; }

            //inline PollLoopPtr & GetPollLoop() { return mLoop; }
            
        private:

            struct epoll_event mEPollEvent;

            //bool mIsClosing;
            //bool mIsClosed;

        public:
            void HandleEvents(struct pollfd const & pollFd);
    };


}
}

#endif

