#ifndef XTNB_TIMER_H
#define XTNB_TIMER_H

#include <XiaoTuNetBox/EventHandler.h>
#include <XiaoTuNetBox/EventLoop.h>

#include <memory>
#include <sys/timerfd.h>


namespace xiaotu {
namespace net {

    class Timer;
    typedef std::shared_ptr<Timer> TimerPtr;
    typedef std::shared_ptr<const Timer> TimerConstPtr;

    class Timer {
        public:
            Timer(Timer const &) = delete;
            Timer & operator = (Timer const &) = delete;
            ~Timer();
            Timer(EventLoop const & loop);

            EventHandlerPtr & GetHandler() { return mEventHandler; }

            typedef std::function<void()> EventCallBk;
            void DisArm();
            void RunAfter(time_t sec, long nsec, EventCallBk cb);
            void RunAfter(const timespec & time, EventCallBk cb);
            void RunEvery(const timespec & time, EventCallBk cb);
            EventCallBk mTimeOutCb;

            void OnReadEvent();
        private:
            EventHandlerPtr mEventHandler;
            struct timespec mOriTime;
            int mFd;
    };

}
}


#endif


