#ifndef XTNB_POLLLOOP_H
#define XTNB_POLLLOOP_H

#include <XiaoTuNetBox/EventHandler.h>
#include <vector>
#include <poll.h>

namespace xiaotu {
namespace net {

    class PollLoop {
        public:
            void LoopOnce(int timeout);

            void AddEventHandler(PollEventHandler *h) { mHandlerList.push_back(h); }
        private:
            std::vector<struct pollfd> mPollFdList;
            std::vector<PollEventHandler*> mHandlerList;
    };

}
}

#endif

