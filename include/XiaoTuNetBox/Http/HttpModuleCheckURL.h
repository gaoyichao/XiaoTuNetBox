#ifndef XTNB_HTTP_MODULE_CHECK_URL_H
#define XTNB_HTTP_MODULE_CHECK_URL_H

#include <XiaoTuNetBox/Http/HttpModule.h>


namespace xiaotu {
namespace net {

    //! @brief 检查请求报文的 URL
    class HttpModuleCheckURL final : public HttpModule {
        public:
            HttpModuleCheckURL(std::string const & workspace)
                : HttpModule(true), mWorkSpace(workspace)
            {}

            //! 目录模组
            HttpModulePtr mDirModule;
            //! 404模组
            HttpModulePtr m404Module;
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
            std::string mWorkSpace;
    };



}
}


#endif
