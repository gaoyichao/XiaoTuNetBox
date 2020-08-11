#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/EventHandler.h>
#include <poll.h>

#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    PollEventHandler::PollEventHandler(int fd) : mLoopIdx(-1) {
        mPollFd.fd = fd;
        mPollFd.events = 0;
        mPollFd.revents = 0;
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

    void PollEventHandler::Apply(PollLoopPtr const & loop) {
        mLoop = loop;
        mLoopIdx = mLoop->Register(this);
        std::cout << "loop idx = " << mLoopIdx << std::endl;
    }

    void PollEventHandler::UnApply() {
        mLoop->UnRegister(this);
        mLoopIdx = -1;
        mLoop.reset();
    }

    void PollEventHandler::HandleEvents(struct pollfd const & pollFd) {
        assert(mPollFd.fd == pollFd.fd);
        assert(mPollFd.events == pollFd.events);

        mPollFd.revents = pollFd.revents;

        if (pollFd.revents & POLLIN) {
            if (mReadCallBk)
                mReadCallBk();
        }
    }

}
}


