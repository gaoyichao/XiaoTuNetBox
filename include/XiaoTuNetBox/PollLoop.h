#ifndef XTNB_POLLLOOP_H
#define XTNB_POLLLOOP_H

#include <XiaoTuNetBox/EventHandler.h>

#include <vector>
#include <poll.h>
#include <memory>

namespace xiaotu {
namespace net {

    class PollLoop;
    typedef std::shared_ptr<PollLoop> PollLoopPtr;
    typedef std::shared_ptr<const PollLoop> PollLoopConstPtr;

    void ApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);
    void UnApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);


    class PollLoop {
        public:
            void LoopOnce(int timeout);

        private:
            int Register(PollEventHandlerPtr const & handler);
            void UnRegister(PollEventHandlerPtr const & handler);

        friend void ApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);
        friend void UnApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);

        private:
            std::vector<int> mIdleIdx;
            std::vector<struct pollfd> mPollFdList;
            std::vector<PollEventHandlerPtr> mHandlerList;
    };

}
}

#endif

