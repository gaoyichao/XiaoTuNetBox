/************************************************************************************
 * 
 * Session
 * 
 ***********************************************************************************/
#ifndef XTNB_SESSION_H
#define XTNB_SESSION_H

#include <XiaoTuNetBox/InBufObserver.h>
#include <XiaoTuNetBox/Connection.h>
#include <memory>

namespace xiaotu {
namespace net {

    class TcpAppServer;
    class Session {
        friend class TcpAppServer;
        public:
            Session(Session const &) = delete;
            Session & operator = (Session const &) = delete;

            virtual char const * ToCString() = 0;

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

            //! @todo 增加输出缓存
            //! 输入缓存 InputBuffer 的观测器
            InBufObserverPtr mInBuf;
    };

    typedef std::shared_ptr<Session> SessionPtr;
    typedef std::weak_ptr<Session> SessionWeakPtr;

}
}


#endif
