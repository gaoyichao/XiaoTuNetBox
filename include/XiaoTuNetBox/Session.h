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
#include <XiaoTuNetBox/WakeUpper.h>
#include <XiaoTuNetBox/EventLoop.h>

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

        protected:
            Session() = default;
            Session(ConnectionPtr const & conn)
            {
                mInBuf = conn->GetInputBuffer().CreateObserver();
            }

            ~Session() = default;

        protected:
            //! 在 TcpAppServer 的会话列表中的索引
            size_t mIdx;

            //! EventLoop 唤醒器
            WakeUpperPtr mWakeUpper;

            //! @todo 增加输出缓存
            //! 输入缓存 InputBuffer 的观测器
            InBufObserverPtr mInBuf;
    };

    typedef std::shared_ptr<Session> SessionPtr;
    typedef std::weak_ptr<Session> SessionWeakPtr;

}
}


#endif
