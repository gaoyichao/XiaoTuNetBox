#include <XiaoTuNetBox/EPollLoop.h>

#include <iostream>
#include <cassert>

#include <unistd.h>

namespace xiaotu {
namespace net {

    EPollLoop::EPollLoop(int max_evs)
        : EventLoop(), mMaxEvs(max_evs), mEvents(max_evs)
    {
        mEpollFd = epoll_create1(EPOLL_CLOEXEC);
    }

    EPollLoop::~EPollLoop()
    {
        std::cout << __FUNCTION__ << std::endl;
        close(mEpollFd);
    }

    EventHandlerPtr EPollLoop::CreateEventHandler(int fd) const
    {
        EPollEventHandlerPtr re = std::make_shared<EPollEventHandler>(fd);
        return std::static_pointer_cast<EventHandler>(re);
    }

    void EPollLoop::LoopOnce(int timeout)
    {
        int nready = epoll_wait(mEpollFd, mEvents.data(), mMaxEvs, -1);
        printf("nready = %d\n", nready);

        //! @todo: -1 == nready
        
        for (int i = 0; i < nready; ++i) {
            EPollEventHandler * ephandler = (EPollEventHandler *)mEvents[i].data.ptr;
            ephandler->HandleEvents(mEvents[i]);
        }
    }

    void EPollLoop::Register(int idx)
    {
        EPollEventHandlerPtr handler = std::static_pointer_cast<EPollEventHandler>(mHandlerList[idx]);

        int re = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, handler->GetFd(), &(handler->mEPollEvent));
        assert(0 == re);
    }

    void EPollLoop::UnRegister(EventHandlerPtr const & h)
    {
        EPollEventHandlerPtr handler = std::static_pointer_cast<EPollEventHandler>(h);

        int re = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, handler->GetFd(), &(handler->mEPollEvent));
    }


}
}


