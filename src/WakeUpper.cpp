#include <XiaoTuNetBox/WakeUpper.h>

#include <functional>
#include <unistd.h>

#include <iostream>

namespace xiaotu {
namespace net {

    WakeUpper::WakeUpper()
    {
        mFd = eventfd(0, EFD_CLOEXEC);
        mEventHandler = PollEventHandlerPtr(new PollEventHandler(mFd));
        mEventHandler->EnableRead(true);
        mEventHandler->SetReadCallBk(std::bind(&WakeUpper::OnReadEvent, this));
    }

    WakeUpper::~WakeUpper()
    {
        close(mFd);
    }

    void WakeUpper::WakeUp(uint64_t u)
    {
        write(mFd, &u, sizeof(u));
    }

    void WakeUpper::OnReadEvent() {
        uint64_t u;
        int nread = read(mFd, &u, sizeof(u));

        std::cout << "wakeup, nread = " << nread << ", u = " << u << std::endl;

        if (mWakeUpCb)
            mWakeUpCb();
    }


}
}

