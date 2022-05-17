#ifndef XTNB_LOOP_H
#define XTNB_LOOP_H

#include <XiaoTuNetBox/Event/EventHandler.h>
#include <XiaoTuNetBox/Event/WakeUpper.h>

#include <atomic>
#include <vector>

namespace xiaotu {
namespace net {

    class EventLoop {
        public:
            EventLoop();
            std::thread::id GetTid() const { return mTid; }
            void WakeUp(uint64_t u);
            void SetWakeUpper(WakeUpperPtr const & ptr) { mWakeUpper = ptr; }

            void Loop(int timeout);
            void PreLoop();
            void StopLoop() { mLooping = false; }

            virtual EventHandlerPtr CreateEventHandler(int fd) const = 0;
            virtual void LoopOnce(int timeout) = 0;
        protected:
            WakeUpperPtr mWakeUpper;
            std::thread::id mTid;
            std::atomic<bool> mLooping;

        public:
            int Add(EventHandlerPtr const & handler);
            void Remove(EventHandlerPtr const & handler);
            virtual void Register(int idx) = 0;
            virtual void UnRegister(EventHandlerPtr const & handler) = 0;
        protected:
            std::vector<int> mIdleIdx;
            std::vector<EventHandlerPtr> mHandlerList;
    };
    typedef std::shared_ptr<EventLoop> EventLoopPtr;
    typedef std::shared_ptr<const EventLoop> EventLoopConstPtr;

    template <typename ObjPtr>
    void ApplyOnLoop(ObjPtr const & obj, EventLoopPtr const & loop) {
        ApplyHandlerOnLoop(obj->GetHandler(), loop);
    }

    template <typename ObjPtr>
    void UnApplyOnLoop(ObjPtr const & obj, EventLoopPtr const & loop) {
        UnApplyHandlerOnLoop(obj->GetHandler(), loop);
    }

    template <typename LoopType>
    std::shared_ptr<LoopType> Create()
    {
        std::shared_ptr<LoopType> loop = std::make_shared<LoopType>();
        WakeUpperPtr wk = std::make_shared<WakeUpper>(*loop);
        ApplyOnLoop(wk, loop);
        loop->SetWakeUpper(wk);
        return loop;
    }

}
}

#endif
