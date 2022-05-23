/************************************************************************************
 * 
 * Session
 * 
 ***********************************************************************************/
#ifndef XTNB_SESSION_H
#define XTNB_SESSION_H

#include <XiaoTuNetBox/Types.h>
#include <XiaoTuNetBox/InBufObserver.h>
#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/Task.h>
#include <XiaoTuNetBox/Event.h>

#include <memory>

namespace xiaotu {
namespace net {

    class TcpAppServer;
    class Session : public Object {
        friend class TcpAppServer;
        public:
            Session(Session const &) = delete;
            Session & operator = (Session const &) = delete;

            virtual char const * ToCString() = 0;

            void BuildWakeUpper(EventLoopPtr const & loop, WakeUpper::EventCallBk cb)
            {
                mWakeUpper = std::make_shared<WakeUpper>(*loop);
                mWakeUpper->SetWakeUpCallBk(std::move(cb));
                ApplyOnLoop(mWakeUpper, loop);
            }

            void ReleaseWakeUpper(EventLoopPtr const & loop)
            {
                UnApplyOnLoop(mWakeUpper, loop);
                mWakeUpper.reset();
            }

            void WakeUp()
            {
                mWakeUpper->WakeUp(4096);
            }

            //! 当前任务
            TaskPtr mCurrTask;
        protected:
            Session() = default;
            ~Session() = default;

        protected:
            //! 在 TcpAppServer 的会话列表中的索引
            size_t mIdx;

            //! EventLoop 唤醒器
            WakeUpperPtr mWakeUpper;
    };

    typedef std::shared_ptr<Session> SessionPtr;
    typedef std::weak_ptr<Session> SessionWeakPtr;

}
}


#endif
