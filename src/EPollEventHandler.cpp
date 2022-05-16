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
        mEPollEvent.data.ptr = this;

        mIsClosing = false;
        mIsClosed = false;
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
