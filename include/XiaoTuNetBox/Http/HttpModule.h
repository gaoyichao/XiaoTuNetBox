#ifndef XTNB_HTTP_MODULE_H
#define XTNB_HTTP_MODULE_H

#include <XiaoTuNetBox/Event.h>
#include <XiaoTuNetBox/Http/HttpHandler.h>

#include <memory>

namespace xiaotu {
namespace net {

    class HttpModule;
    typedef std::shared_ptr<HttpModule> HttpModulePtr;

    class HttpModule {
        public:
            HttpModule(bool immediately = false)
                : mImmediately(immediately)
            {}

            bool Handle(HttpHandlerWeakPtr const & handler);
            void SetSuccessModule(HttpModulePtr const & module) { mSuccessModule = module; }
            void SetFailureModule(HttpModulePtr const & module) { mFailureModule = module; }

        protected:
            //! @brief 实际的处理过程
            virtual bool Process(HttpHandlerWeakPtr const & handler) = 0;

        protected:
            //! Process 成功后的处理模块索引
            HttpModulePtr mSuccessModule;
            //! Process 失败后的处理模块索引
            HttpModulePtr mFailureModule;

            bool mImmediately;
    };

    //! @brief HTTP 处理非法请求报文的模块
    class HttpModuleInvalidRequest final : public HttpModule {
        public:
            HttpModuleInvalidRequest()
                : HttpModule(true)
            {}
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
    };

    //! @brief HTTP 解析请求报文中 Cookie 的模块
    class HttpModuleParseCookie final : public HttpModule {
        public:
            HttpModuleParseCookie()
                : HttpModule(true)
            {}
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
    };


    //! @brief HTTP 发送响应报文的模块
    class HttpModuleResponse final : public HttpModule {
        public:
            HttpModuleResponse()
                : HttpModule(true)
            {}
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
    };

    //! @brief HTTP 检查请求方法的模块
    class HttpModuleCheckMethod final : public HttpModule {
        public:
            HttpModuleCheckMethod()
                : HttpModule(true)
            {}

            HttpModulePtr mOnHeadModule;
            HttpModulePtr mOnGetModule;
            HttpModulePtr mOnPostModule;
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
    };
    typedef std::shared_ptr<HttpModuleCheckMethod> HttpModuleCheckMethodPtr;

    //! @brief HTTP 发送响应报文的模块
    class HttpModuleUnSupport final : public HttpModule {
        public:
            HttpModuleUnSupport()
                : HttpModule(true)
            {}
        private:
            virtual bool Process(HttpHandlerWeakPtr const & handler) override;
    };


}
}

#endif
