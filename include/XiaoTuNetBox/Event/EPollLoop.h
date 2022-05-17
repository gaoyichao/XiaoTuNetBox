#ifndef XTNB_EPOLL_LOOP_H
#define XTNB_EPOLL_LOOP_H

#include <XiaoTuNetBox/Event/EventLoop.h>
#include <XiaoTuNetBox/Event/EPollEventHandler.h>

namespace xiaotu {
namespace net {

    class EPollLoop;
    typedef std::shared_ptr<EPollLoop> EPollLoopPtr;
    typedef std::shared_ptr<const EPollLoop> EPollLoopConstPtr;

    class EPollLoop : public EventLoop {
        friend class EPollEventHandler;
        public:
            EPollLoop(int max_evs = 10);
            EPollLoop(EPollLoop const &) = delete;
            EPollLoop & operator = (EPollLoop const &) = delete;
            ~EPollLoop();

            virtual EventHandlerPtr CreateEventHandler(int fd) const override;
            virtual void LoopOnce(int timeout) override;

        private:
            virtual void Register(int idx) override;
            virtual void UnRegister(EventHandlerPtr const & h) override;

        public:
            int GetEpollFd() const { return mEpollFd; }
        private:
            int mEpollFd;
            int mMaxEvs;
            std::vector<struct epoll_event> mEvents;
    };
}
}

#endif

