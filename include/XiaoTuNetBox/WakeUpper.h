#ifndef XTNB_WAKEUPPER_H
#define XTNB_WAKEUPPER_H

#include <XiaoTuNetBox/EventHandler.h>

#include <memory>

namespace xiaotu {
namespace net {

    class EventLoop;
    class WakeUpper;
    typedef std::shared_ptr<WakeUpper> WakeUpperPtr;
    typedef std::shared_ptr<const WakeUpper> WakeUpperConstPtr;

    template <typename LoopType>
    WakeUpperPtr NewWakeUpper()
    {
        return nullptr;
    }

    class WakeUpper {
        public:
            WakeUpper(EventLoop & loop);
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
