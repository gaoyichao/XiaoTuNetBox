#ifndef XTNB_EPOLL_EVENT_HANDLER_H
#define XTNB_EPOLL_EVENT_HANDLER_H

#include <XiaoTuNetBox/Event/EventHandler.h>

#include <sys/epoll.h>
#include <iostream>

namespace xiaotu {
namespace net {

    class EPollEventHandler;
    typedef std::shared_ptr<EPollEventHandler> EPollEventHandlerPtr;
    typedef std::shared_ptr<const EPollEventHandler> EPollEventHandlerConstPtr;


    class EPollLoop;
    class EPollEventHandler : public EventHandler {
        typedef std::shared_ptr<EPollLoop> EPollLoopPtr;

        friend class EPollLoop;
        public:
            EPollEventHandler(int fd);
            EPollEventHandler(EPollEventHandler const &) = delete;
            EPollEventHandler & operator = (EPollEventHandler const &) = delete;

            void UseEdgeTrigger(bool en);

            virtual void SetClosing(bool en) override;
            virtual void EnableRead(bool en) override;
            virtual void EnableWrite(bool en) override;

            //void DisableAll() { mPollFd.events = 0; }
            //bool IsReading() const { return (mPollFd.events & POLLIN); }
            //bool IsWriting() const { return (mPollFd.events & POLLOUT); }
            //struct pollfd const & GetPollFd() const { return mPollFd; }

            void HandleEvents(struct epoll_event const & event);
            EPollLoopPtr GetEPollLoop();
        private:
            void UpdateEpoll();
        private:
            struct epoll_event mEPollEvent;
            bool mIsClosing;
            bool mIsClosed;
    };


}
}

#endif

