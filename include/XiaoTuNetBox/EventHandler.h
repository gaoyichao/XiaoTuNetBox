#ifndef XTNB_EVENTHANDLER_H
#define XTNB_EVENTHANDLER_H

#include <functional>
#include <poll.h>

namespace xiaotu {
namespace net {

    class PollEventHandler {
        public:
            PollEventHandler(int fd);
            PollEventHandler(PollEventHandler const &) = delete;
            PollEventHandler & operator = (PollEventHandler const &) = delete;

            void EnableRead(bool en);
            void EnableWrite(bool en);
            void DisableAll() { mPollFd.events = 0; }
            bool IsReading() const { return (mPollFd.events & POLLIN); }
            bool IsWriting() const { return (mPollFd.events & POLLOUT); }
            struct pollfd const & GetPollFd() const { return mPollFd; }

            void HandleEvents(struct pollfd const & pollFd);

            typedef std::function<void()> EventCallBk;
            void SetReadCallBk(EventCallBk cb) { mReadCallBk = std::move(cb); }

        private:
            struct pollfd mPollFd;
            EventCallBk mReadCallBk;

    };

}
}


#endif

