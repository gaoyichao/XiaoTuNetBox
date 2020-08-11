#include <XiaoTuNetBox/PollLoop.h>

#include <iostream>
#include <cassert>

namespace xiaotu {
namespace net {

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

    int PollLoop::Register(PollEventHandler* handler) {
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

    void PollLoop::UnRegister(PollEventHandler* handler) {
        int idx = handler->GetLoopIdx();

        assert(idx >= 0);
        assert(idx < mPollFdList.size() && mPollFdList.size() == mHandlerList.size());
        assert(mPollFdList[idx].fd == handler->GetFd());

        mPollFdList[idx].fd = -1;
        mPollFdList[idx].events = 0;
        mPollFdList[idx].revents = 0;

        mHandlerList[idx] = NULL;
        mIdleIdx.push_back(idx);
    }



}
}


