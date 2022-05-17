#include <XiaoTuNetBox/Event/PollLoop.h>
#include <XiaoTuNetBox/Event/PollEventHandler.h>

#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    PollEventHandler::PollEventHandler(int fd)
        : EventHandler(fd)
    {
        mPollFd.fd = fd;
        mPollFd.events = 0;
        mPollFd.revents = 0;

        mIsClosing = false;
        mIsClosed = false;
    }


    void PollEventHandler::EnableRead(bool en) {
        if (en)
            mPollFd.events |= POLLIN;
        else
            mPollFd.events &= ~POLLIN;
    }

    void PollEventHandler::EnableWrite(bool en) {
        if (en)
            mPollFd.events |= POLLOUT;
        else
            mPollFd.events &= ~POLLOUT;
    }

    void PollEventHandler::SetClosing(bool en) {
        mIsClosing = en;
    }

    void PollEventHandler::HandleEvents(struct pollfd const & pollFd) {
        assert(mPollFd.fd == pollFd.fd);
        mPollFd.revents = pollFd.revents;

        if (mPollFd.revents & POLLIN) {
            if (mReadCallBk)
                mReadCallBk();
        }

        if ((mPollFd.events & POLLOUT) || (mPollFd.revents & POLLOUT)) {
            if (mWriteCallBk)
                mWriteCallBk();
        }

        if (mIsClosing && !mIsClosed) {
            if (mClosingCallBk)
                mClosingCallBk();
            mIsClosed = true;
        }
    }

    PollLoopPtr PollEventHandler::GetPollLoop()
    {
        return std::static_pointer_cast<PollLoop>(mLoop);
    }
}
}


