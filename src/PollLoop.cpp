#include <XiaoTuNetBox/PollLoop.h>

#include <iostream>
#include <cassert>

namespace xiaotu {
namespace net {
    EventHandlerPtr PollLoop::CreateEventHandler(int fd) const
    {
        PollEventHandlerPtr re = std::make_shared<PollEventHandler>(fd);
        return std::static_pointer_cast<EventHandler>(re);
    }

    void PollLoop::LoopOnce(int timeout) {
        int nready = poll(mPollFdList.data(), mPollFdList.size(), timeout);

        for (int i = 0; i < mPollFdList.size(); i++) {
            if (mPollFdList[i].fd < 0) {
                mHandlerList[i] = NULL;
                continue;
            }
            PollEventHandlerPtr handler = std::static_pointer_cast<PollEventHandler>(mHandlerList[i]);
            handler->HandleEvents(mPollFdList[i]);
        }
    }

    void PollLoop::Register(int idx) {
        PollEventHandlerPtr handler = std::static_pointer_cast<PollEventHandler>(mHandlerList[idx]);
        if (idx == mPollFdList.size()) {
            mPollFdList.push_back(handler->GetPollFd());
        } else {
            mPollFdList[idx] = handler->GetPollFd();
        }
    }

    void PollLoop::UnRegister(EventHandlerPtr const & h) {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        PollEventHandlerPtr handler = std::static_pointer_cast<PollEventHandler>(h);
        int idx = handler->GetLoopIdx();

        assert(idx >= 0 &&  idx < mPollFdList.size());
        assert(mPollFdList[idx].fd == handler->GetFd());

        mPollFdList[idx].fd = -1;
        mPollFdList[idx].events = 0;
        mPollFdList[idx].revents = 0;
    }


}
}


