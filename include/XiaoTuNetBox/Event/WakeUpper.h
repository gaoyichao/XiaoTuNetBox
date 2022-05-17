#ifndef XTNB_WAKEUPPER_H
#define XTNB_WAKEUPPER_H

#include <XiaoTuNetBox/Event/EventHandler.h>

namespace xiaotu {
namespace net {

    class EventLoop;
    class WakeUpper;
    typedef std::shared_ptr<WakeUpper> WakeUpperPtr;
    typedef std::shared_ptr<const WakeUpper> WakeUpperConstPtr;

    class WakeUpper {
        public:
            WakeUpper(EventLoop const & loop);
            WakeUpper(WakeUpper const &) = delete;
            WakeUpper & operator = (WakeUpper const &) = delete;
            ~WakeUpper();

            EventHandlerPtr & GetHandler() { return mEventHandler; }
            void WakeUp(uint64_t u);

            typedef std::function<void()> EventCallBk;
            void SetWakeUpCallBk(EventCallBk cb) { mWakeUpCb = std::move(cb); }
            EventCallBk mWakeUpCb;

            void OnReadEvent();
        private:
            EventHandlerPtr mEventHandler;
            int mFd;
    };

}
}


#endif
