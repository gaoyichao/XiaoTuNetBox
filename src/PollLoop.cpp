#include <XiaoTuNetBox/PollLoop.h>

namespace xiaotu {
namespace net {

    void PollLoop::LoopOnce(int timeout) {
        int nready = poll(mPollFdList.data(), mPollFdList.size(), timeout);

        for (int i = 0; i < mPollFdList.size(); i++) {
            if (mPollFdList[i].fd < 0)
                continue;
            mHandlerList[i]->HandleEvents(mPollFdList[i]);
        }
    }


}
}


