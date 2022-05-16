#include <XiaoTuNetBox/EventHandler.h>
#include <XiaoTuNetBox/EventLoop.h>

#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    bool EventHandler::SetNonBlock(bool en)
    {
        int fl = fcntl(mFd, F_GETFL);

        if (en)
            fl |= O_NONBLOCK;
        else
            fl &= ~O_NONBLOCK;

        if (-1 == fcntl(mFd, F_SETFL, fl)) {
            perror("修改NONBLOCK出错");
            return false;
        }
        return true;
    }

    std::thread::id EventHandler::GetLoopTid() const
    {
        assert(mLoop);

        return mLoop->GetTid();
    }

    void EventHandler::WakeUpLoop() {
        assert(mLoop);

        uint64_t idx = mLoopIdx;
        mLoop->WakeUp(idx);
    }



}
}


