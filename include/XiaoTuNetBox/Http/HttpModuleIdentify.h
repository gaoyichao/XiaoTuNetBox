#ifndef XTNB_HTTP_MODULE_IDENTIFY_H
#define XTNB_HTTP_MODULE_IDENTIFY_H

#include <XiaoTuNetBox/Http/HttpModule.h>


namespace xiaotu {
namespace net {

    //! @brief HTTP 基础验证
    class HttpModuleBasicIdentify final : public HttpModule {
        public:
            HttpModuleBasicIdentify()
                : HttpModule(true)
            {}

            bool CheckId(HttpRequest const & request);
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
    };



}
}


#endif
