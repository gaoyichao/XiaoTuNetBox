#ifndef XTNB_EPOLL_LOOP_H
#define XTNB_EPOLL_LOOP_H

#include <XiaoTuNetBox/EventLoop.h>
#include <XiaoTuNetBox/EPollEventHandler.h>

#include <vector>
#include <memory>
#include <sys/epoll.h>

namespace xiaotu {
namespace net {

    class EPollLoop;
    typedef std::shared_ptr<EPollLoop> EPollLoopPtr;
    typedef std::shared_ptr<const EPollLoop> EPollLoopConstPtr;

    class EPollLoop : public EventLoop {
        public:
            EPollLoop();
            EPollLoop(EPollLoop const &) = delete;
            EPollLoop & operator = (EPollLoop const &) = delete;
            ~EPollLoop();

            virtual EventHandlerPtr CreateEventHandler(int fd) override;
            virtual void LoopOnce(int timeout) override;

        private:
            virtual void Register(int idx) override;
            virtual void UnRegister(EventHandlerPtr const & h) override;

        public:
            int GetEpollFd() const { return mEpollFd; }
        private:
            int mEpollFd;
            //std::vector<int> mIdleIdx;
            //std::vector<PollEventHandlerPtr> mHandlerList;
            //std::vector<struct pollfd> mPollFdList;
    };
}
}

#endif

