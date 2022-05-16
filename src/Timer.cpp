#include <XiaoTuNetBox/Timer.h>

#include <functional>
#include <unistd.h>

#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    Timer::Timer(EventLoop const & loop) {
        mFd = timerfd_create(CLOCK_BOOTTIME, TFD_NONBLOCK | TFD_CLOEXEC);
        assert(mFd > 0);

        mEventHandler = loop.CreateEventHandler(mFd);
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&Timer::OnReadEvent, this));
    }

    Timer::~Timer() {
        close(mFd);
    }


    void Timer::RunAfter(time_t sec, long nsec, EventCallBk cb) {
        struct timespec t;
        t.tv_sec = sec;
        t.tv_nsec = nsec;

        RunAfter(t, cb);
    }

    void Timer::DisArm() {
        struct itimerspec new_value;

        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_nsec = 0;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_nsec = 0;

		if (timerfd_settime(mFd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &new_value, NULL) == -1) {
            std::cout << errno << std::endl;
            perror("timerfd_settime");
            exit(1);
		}
    }

    void Timer::RunAfter(const timespec & time, EventCallBk cb) {
        if (clock_gettime(CLOCK_BOOTTIME, &mOriTime) == -1) {
            perror("clock_gettime failed!");
            exit(1);
        }

        struct itimerspec new_value;
        long long nsec = mOriTime.tv_nsec + time.tv_nsec;
        time_t _sec = nsec / 1000000000;
        long _nsec = nsec % 1000000000;

        new_value.it_value.tv_sec = mOriTime.tv_sec + time.tv_sec + _sec;
        new_value.it_value.tv_nsec = _nsec;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_nsec = 0;

		if (timerfd_settime(mFd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &new_value, NULL) == -1) {
            std::cout << errno << std::endl;
            perror("timerfd_settime");
            exit(1);
		}

        mTimeOutCb = std::move(cb);
    }

    void Timer::RunEvery(const timespec & time, EventCallBk cb) {
        if (clock_gettime(CLOCK_BOOTTIME, &mOriTime) == -1) {
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

        printf("0x%lx\n", exp);

        if (mTimeOutCb) {
            mTimeOutCb();
        }
    }

}
}

