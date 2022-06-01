#ifndef XTNB_HTTP_MODULE_GET_H
#define XTNB_HTTP_MODULE_GET_H

#include <XiaoTuNetBox/Http/HttpModule.h>


namespace xiaotu {
namespace net {

    //! @brief HTTP 基础验证
    class HttpModuleLoadFile final : public HttpModule {
        static const int mDefaultLoadSize;

        public:
            HttpModuleLoadFile()
                : HttpModule(true)
            {}

        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
            bool NextLoad(ConnectionWeakPtr const & conptr);
    };



}
}


#endif
