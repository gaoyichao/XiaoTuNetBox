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
    PollLoopPtr CreatePollLoop();

    class PollLoop {
        friend PollLoopPtr CreatePollLoop();
        public:
            PollLoop(PollLoop const &) = delete;
            PollLoop & operator = (PollLoop const &) = delete;

            void WakeUp(uint64_t u);

            pid_t GetTid() const { return mTid; }

            void Loop(int timeout);
            /*
             * 慎用
             */
            void PreLoop();
            void LoopOnce(int timeout);

        private:
            PollLoop();
            int Register(PollEventHandlerPtr const & handler);
            void UnRegister(PollEventHandlerPtr const & handler);

            void OnWakeUp();

        friend void ApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);
        friend void UnApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop);

        private:
            std::vector<int> mIdleIdx;
            std::vector<struct pollfd> mPollFdList;
            std::vector<PollEventHandlerPtr> mHandlerList;
            PollEventHandlerPtr mWakeUpHandler;

            pid_t mTid;
            bool mLooping;
    };


}
}

#endif

