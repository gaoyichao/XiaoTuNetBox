#ifndef XTNB_HTTP_MODULE_UPGRADE_WS_H
#define XTNB_HTTP_MODULE_UPGRADE_WS_H

#include <XiaoTuNetBox/Http/HttpModule.h>
#include <XiaoTuNetBox/WebSocketServer.h>


namespace xiaotu {
namespace net {

    //! @brief 检查请求报文的 URL
    class HttpModuleUpgradeWs final : public HttpModule {
        public:
            HttpModuleUpgradeWs(WebSocketServer * ws)
                : HttpModule(true), mWs(ws)
            {}

            HttpModulePtr mHandShakeFailed;
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
            WebSocketServer * mWs;
    };



}
}


#endif
