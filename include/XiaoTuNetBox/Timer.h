#ifndef XTNB_TIMER_H
#define XTNB_TIMER_H

#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/EventHandler.h>

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
            Timer();

            PollEventHandlerPtr & GetHandler() { return mEventHandler; }

            typedef std::function<void()> EventCallBk;
            void RunAfter(const timespec & time, EventCallBk cb);
            void RunEvery(const timespec & time, EventCallBk cb);
            EventCallBk mTimeOutCb;

            void OnReadEvent();
        private:
            PollEventHandlerPtr mEventHandler;
            struct timespec mOriTime;
            int mFd;
    };

}
}


#endif


