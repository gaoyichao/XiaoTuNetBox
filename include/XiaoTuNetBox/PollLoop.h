#ifndef XTNB_POLLLOOP_H
#define XTNB_POLLLOOP_H

#include <XiaoTuNetBox/EventHandler.h>

#include <vector>
#include <poll.h>
#include <memory>

namespace xiaotu {
namespace net {

    class PollLoop {
        public:
            void LoopOnce(int timeout);

            int Register(PollEventHandler* handler);
            void UnRegister(PollEventHandler* handler);

        private:
            std::vector<int> mIdleIdx;
            std::vector<struct pollfd> mPollFdList;
            std::vector<PollEventHandler*> mHandlerList;
    };
    typedef std::shared_ptr<PollLoop> PollLoopPtr;
    typedef std::shared_ptr<const PollLoop> PollLoopConstPtr;


}
}

#endif

