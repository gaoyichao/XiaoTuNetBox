#ifndef XTNB_HTTP_MODULE_LOGIN_H
#define XTNB_HTTP_MODULE_LOGIN_H

#include <XiaoTuNetBox/Http/HttpModule.h>

#include <string>
#include <memory>
#include <set>

namespace xiaotu {
namespace net {

    //! @brief HTTP 登录认证
    class HttpModuleLogin final : public HttpModule {
        public:
            HttpModuleLogin()
                : HttpModule(true)
            {}

            std::string mLoginHtml;
        private:
            bool FailureReturn(HttpHandlerPtr const &h, HttpResponsePtr const & response);
            bool CheckRequest(HttpRequest const & request, HttpResponse & response);
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
            std::set<std::string> mRefererList;
    };
    typedef std::shared_ptr<HttpModuleLogin> HttpModuleLoginPtr;



}
}


#endif
