#include <XiaoTuNetBox/EventHandler.h>
#include <poll.h>

#include <cassert>

namespace xiaotu {
namespace net {

    PollEventHandler::PollEventHandler(int fd) {
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


