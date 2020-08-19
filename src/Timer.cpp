#include <XiaoTuNetBox/Timer.h>

#include <sys/timerfd.h>
#include <functional>
#include <unistd.h>

#include <iostream>

namespace xiaotu {
namespace net {

    Timer::Timer() {
        mFd = timerfd_create(CLOCK_REALTIME, 0);
        if (mFd < 0) {
            perror("创建Socket失败");
            exit(1);
        }
        mEventHandler = PollEventHandlerPtr(new PollEventHandler(mFd));
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&Timer::OnReadEvent, this));
        std::cout << "mFd = " << mFd << std::endl;
    }

    Timer::~Timer() {
        close(mFd);
    }

    void Timer::RunAfter(const timespec & time, EventCallBk cb) {
        if (clock_gettime(CLOCK_REALTIME, &mOriTime) == -1) {
            perror("clock_gettime failed!");
            exit(1);
        }

        struct itimerspec new_value;
        new_value.it_value.tv_sec = mOriTime.tv_sec + time.tv_sec;
        new_value.it_value.tv_nsec = mOriTime.tv_nsec + time.tv_nsec;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_nsec = 0;

		if (timerfd_settime(mFd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
            std::cout << errno << std::endl;
            perror("timerfd_settime");
            exit(1);
		}

        mTimeOutCb = std::move(cb);
    }

    void Timer::RunEvery(const timespec & time, EventCallBk cb) {
        if (clock_gettime(CLOCK_REALTIME, &mOriTime) == -1) {
            perror("clock_gettime failed!");
            exit(1);
        }

        struct itimerspec new_value;
        new_value.it_value.tv_sec = mOriTime.tv_sec;
        new_value.it_value.tv_nsec = mOriTime.tv_nsec;
        new_value.it_interval.tv_sec = time.tv_sec;
        new_value.it_interval.tv_nsec = time.tv_nsec;

		if (timerfd_settime(mFd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
            std::cout << errno << std::endl;
            perror("timerfd_settime");
            exit(1);
		}

        mTimeOutCb = std::move(cb);
    }

    void Timer::OnReadEvent() {
        uint64_t exp;
        ssize_t s = read(mFd, &exp, sizeof(exp));

        if (mTimeOutCb) {
            mTimeOutCb();
        }
    }

}
}

