#ifndef XTNB_POLLLOOP_H
#define XTNB_POLLLOOP_H

#include <XiaoTuNetBox/Event/EventLoop.h>
#include <XiaoTuNetBox/Event/PollEventHandler.h>

namespace xiaotu {
namespace net {

    class PollLoop;
    typedef std::shared_ptr<PollLoop> PollLoopPtr;
    typedef std::shared_ptr<const PollLoop> PollLoopConstPtr;

    class PollLoop : public EventLoop {
        public:
            PollLoop() : EventLoop() {};
            PollLoop(PollLoop const &) = delete;
            PollLoop & operator = (PollLoop const &) = delete;

            virtual EventHandlerPtr CreateEventHandler(int fd) const override;
            virtual void LoopOnce(int timeout) override;

        private:
            virtual void Register(int idx) override;
            virtual void UnRegister(EventHandlerPtr const & h) override;

        private:
            std::vector<struct pollfd> mPollFdList;
    };
}
}

#endif

