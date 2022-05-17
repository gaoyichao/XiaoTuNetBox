#include <XiaoTuNetBox/Event/EPollEventHandler.h>
#include <XiaoTuNetBox/Event/EPollLoop.h>

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
        mEPollEvent.events = 0;

        mIsClosing = false;
        mIsClosed = false;
    }

    void EPollEventHandler::UpdateEpoll()
    {
        if (nullptr != mLoop) {
            EPollLoopPtr epoll = std::static_pointer_cast<EPollLoop>(mLoop);
            int re = epoll_ctl(epoll->mEpollFd, EPOLL_CTL_MOD, mFd, &(mEPollEvent));
            assert(0 == re);
        }
    }

    void EPollEventHandler::EnableRead(bool en) {
        if (en)
            mEPollEvent.events |= EPOLLIN;
        else
            mEPollEvent.events &= ~EPOLLIN;
        UpdateEpoll();
    }

    void EPollEventHandler::EnableWrite(bool en) {
        if (en)
            mEPollEvent.events |= EPOLLOUT;
        else
            mEPollEvent.events &= ~EPOLLOUT;
        UpdateEpoll();
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
        UpdateEpoll();
    }

    void EPollEventHandler::SetClosing(bool en) {
        mIsClosing = en;
    }

    EPollLoopPtr EPollEventHandler::GetEPollLoop()
    {
        return std::static_pointer_cast<EPollLoop>(mLoop);
    }

    void EPollEventHandler::HandleEvents(struct epoll_event const & event)
    {
        uint32_t ev = event.events;

        if (ev & EPOLLIN) {
            if (mReadCallBk)
                mReadCallBk();
        }

        if ((mEPollEvent.events & EPOLLOUT) || (ev & EPOLLOUT)) {
            if (mWriteCallBk)
                mWriteCallBk();
        }

        if (mIsClosing && !mIsClosed) {
            if (mClosingCallBk)
                mClosingCallBk();
            mIsClosed = true;
        }

    }

}
}
