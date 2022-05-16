#include <XiaoTuNetBox/EPollLoop.h>

#include <iostream>
#include <cassert>

#include <unistd.h>

namespace xiaotu {
namespace net {

    EPollLoop::EPollLoop()
        : EventLoop()
    {
        mEpollFd = epoll_create1(EPOLL_CLOEXEC);
    }

    EPollLoop::~EPollLoop()
    {
        std::cout << __FUNCTION__ << std::endl;
        close(mEpollFd);
    }

    EventHandlerPtr EPollLoop::CreateEventHandler(int fd)
    {
        EPollEventHandlerPtr re = std::make_shared<EPollEventHandler>(fd);
        return std::static_pointer_cast<EventHandler>(re);
    }

    void EPollLoop::LoopOnce(int timeout)
    {
        //int nready = poll(mPollFdList.data(), mPollFdList.size(), timeout);

        //for (int i = 0; i < mPollFdList.size(); i++) {
        //    if (mPollFdList[i].fd < 0) {
        //        mHandlerList[i] = NULL;
        //        continue;
        //    }
        //    mHandlerList[i]->HandleEvents(mPollFdList[i]);
        //}
    }


    void EPollLoop::Register(int idx) {
        EPollEventHandlerPtr handler = std::static_pointer_cast<EPollEventHandler>(mHandlerList[idx]);

    }

    void EPollLoop::UnRegister(EventHandlerPtr const & h) {
        EPollEventHandlerPtr handler = std::static_pointer_cast<EPollEventHandler>(h);
    }


}
}


