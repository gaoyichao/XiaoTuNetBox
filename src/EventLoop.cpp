#include <XiaoTuNetBox/EventLoop.h>

#include <iostream>
#include <cassert>

namespace xiaotu {
namespace net {

    void ApplyHandlerOnLoop(EventHandlerPtr const & h, EventLoopPtr const & loop) {
        int idx = loop->Add(h);
        loop->Register(idx);

        h->mLoopIdx = idx;
        h->mLoop = loop;
        std::cout << "apply loop idx = " << h->mLoopIdx << std::endl;
    }

    void UnApplyHandlerOnLoop(EventHandlerPtr const & h, EventLoopPtr const & loop) {
        std::cout << "unapply loop idx = " << h->mLoopIdx << std::endl;
        loop->UnRegister(h);
        loop->Remove(h);

        h->mLoopIdx = -1;
        h->mLoop.reset();
    }

    EventLoop::EventLoop() {
    }

    int EventLoop::Add(EventHandlerPtr const & handler)
    {
        if (mIdleIdx.empty()) {
            mHandlerList.push_back(handler);
            return (mHandlerList.size() - 1);
        } else {
            int idx = mIdleIdx.back();
            mIdleIdx.pop_back();

            mHandlerList[idx] = handler;
            return idx;
        }
    }

    void EventLoop::Remove(EventHandlerPtr const & handler)
    {
        int idx = handler->GetLoopIdx();

        assert(idx >= 0 && idx < mHandlerList.size());
        assert(mHandlerList[idx] == handler);

        mHandlerList[idx].reset();
        mIdleIdx.push_back(idx);
    }

    void EventLoop::WakeUp(uint64_t u) {
        assert(mTid != std::this_thread::get_id());
        mWakeUpper->WakeUp(u);
    }

    void EventLoop::PreLoop() {
        mTid = std::this_thread::get_id();
        mLooping = true;
    }

    void EventLoop::Loop(int timeout) {
        PreLoop();
        while (mLooping) {
            LoopOnce(timeout);
        }
    }

}
}


