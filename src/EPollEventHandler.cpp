#include <XiaoTuNetBox/EPollEventHandler.h>
#include <XiaoTuNetBox/EPollLoop.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    EPollEventHandler::EPollEventHandler(int fd)
        : EventHandler(fd)
    {
    }

    void EPollEventHandler::EnableRead(bool en) {
        if (en)
            mEPollEvent.events |= EPOLLIN;
        else
            mEPollEvent.events &= ~EPOLLIN;
    }

    void EPollEventHandler::EnableWrite(bool en) {
        if (en)
            mEPollEvent.events |= EPOLLOUT;
        else
            mEPollEvent.events &= ~EPOLLOUT;
    }

    void EPollEventHandler::UseEdgeTrigger(bool en)
    {
        if (en) {
            bool re = SetNonBlock(true);
            assert(re);
            mEPollEvent.events |= EPOLLET;
        } else {
            mEPollEvent.events &= ~EPOLLET;
        }
    }


}
}
