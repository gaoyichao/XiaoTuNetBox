#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/EventHandler.h>

#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    PollEventHandler::PollEventHandler(int fd) : mLoopIdx(-1) {
        mPollFd.fd = fd;
        mPollFd.events = 0;
        mPollFd.revents = 0;

        mIsClosing = false;
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

    bool PollEventHandler::SetNonBlock(bool en) {
        int fl = fcntl(mPollFd.fd, F_GETFL);

        if (en)
            fl |= O_NONBLOCK;
        else
            fl &= ~O_NONBLOCK;

        if (-1 == fcntl(mPollFd.fd, F_SETFL, fl)) {
            perror("修改NONBLOCK出错");
            return false;
        }
        return true;
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

        if (mIsClosing) {
            if (mClosingCallBk)
                mClosingCallBk();
        }
    }

    pid_t PollEventHandler::GetLoopTid() const {
        assert(mLoop);

        return mLoop->GetTid();
    }

    void PollEventHandler::WakeUpLoop() {
        assert(mLoop);

        uint64_t idx = mLoopIdx;
        mLoop->WakeUp(idx);
    }

}
}


