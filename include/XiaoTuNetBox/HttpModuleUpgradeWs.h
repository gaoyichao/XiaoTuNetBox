#ifndef XTNB_HTTP_MODULE_UPGRADE_WS_H
#define XTNB_HTTP_MODULE_UPGRADE_WS_H

#include <XiaoTuNetBox/Http/HttpModule.h>
#include <XiaoTuNetBox/WebSocketServer.h>

#include <functional>

namespace xiaotu {
namespace net {

    //! @brief 检查请求报文的 URL
    class HttpModuleUpgradeWs final : public HttpModule {
        public:
            typedef std::function<void(ConnectionPtr const & con,
                                       HttpHandlerPtr const & h,
                                       WebSocketHandlerPtr const & wsh)> HandlerCallBk;
        public:
            HttpModuleUpgradeWs(HandlerCallBk cb)
                : HttpModule(true)
            {
                mHandlerCallBk = std::move(cb);
            }

            HttpModulePtr mHandShakeFailed;
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
            HandlerCallBk mHandlerCallBk;
    };



}
}


#endif
