#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/ThreadTools.h>

#include <sys/eventfd.h>
#include <iostream>
#include <cassert>

namespace xiaotu {
namespace net {

    PollLoopPtr CreatePollLoop() {
        PollLoopPtr loop = PollLoopPtr(new PollLoop);
        ApplyHandlerOnLoop(loop->mWakeUpHandler, loop);
        return loop;
    }

    PollLoop::PollLoop()
        : mTid(0), mLooping(false)
    {
        mWakeUpHandler = PollEventHandlerPtr(new PollEventHandler(eventfd(0, EFD_CLOEXEC)));
        mWakeUpHandler->EnableRead(true);
        mWakeUpHandler->SetReadCallBk(std::bind(&PollLoop::OnWakeUp, this));
    }

    void PollLoop::WakeUp(uint64_t u) {
        assert(0 != mTid);
        assert(mTid != ThreadTools::GetCurrentTid());

        int md = mWakeUpHandler->GetFd();
        int s = write(md, &u, sizeof(u));
    }

    void PollLoop::OnWakeUp() {
        uint64_t u;
        int md = mWakeUpHandler->GetFd();
        int nread = read(md, &u, sizeof(u));
        std::cout << "poll loop wakeup, nread = " << nread << ", u = " << u << std::endl;
    }

    void PollLoop::Loop(int timeout) {
        assert(0 == mTid);
        mTid = ThreadTools::GetCurrentTid();
        mLooping = true;

        while (mLooping) {
            LoopOnce(timeout);
        }

        mTid = 0;
    }

    void PollLoop::LoopOnce(int timeout) {
        int nready = poll(mPollFdList.data(), mPollFdList.size(), timeout);
        std::cout << "nready = " << nready << std::endl;

        for (int i = 0; i < mPollFdList.size(); i++) {
            if (mPollFdList[i].fd < 0) {
                mHandlerList[i] = NULL;
                continue;
            }
            mHandlerList[i]->HandleEvents(mPollFdList[i]);
        }
    }

    int PollLoop::Register(PollEventHandlerPtr const & handler) {
        if (mIdleIdx.empty()) {
            mPollFdList.push_back(handler->GetPollFd());
            mHandlerList.push_back(handler);
            return (mHandlerList.size() - 1);
        } else {
            int idx = mIdleIdx.back();
            std::cout << "register idx:" << idx << std::endl;
            mIdleIdx.pop_back();

            mPollFdList[idx] = handler->GetPollFd();
            mHandlerList[idx] = handler;
            return idx;
        }
    }

    void PollLoop::UnRegister(PollEventHandlerPtr const & handler) {
        int idx = handler->GetLoopIdx();

        assert(idx >= 0);
        assert(idx < mPollFdList.size() && mPollFdList.size() == mHandlerList.size());
        assert(mPollFdList[idx].fd == handler->GetFd());
        assert(mHandlerList[idx] == handler);

        mPollFdList[idx].fd = -1;
        mPollFdList[idx].events = 0;
        mPollFdList[idx].revents = 0;

        mHandlerList[idx].reset();
        mIdleIdx.push_back(idx);
    }

    void ApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop) {
        h->mLoop = loop;
        h->mLoopIdx = loop->Register(h);
        std::cout << "apply loop idx = " << h->mLoopIdx << std::endl;
    }


    void UnApplyHandlerOnLoop(PollEventHandlerPtr const & h, PollLoopPtr const & loop) {
        std::cout << "unapply loop idx = " << h->mLoopIdx << std::endl;
        loop->UnRegister(h);
        h->mLoopIdx = -1;
        h->mLoop.reset();
    }



}
}


