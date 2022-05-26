/************************************************************************************
 * 
 * HttpSession - 一次Http会话
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SESSION_H
#define XTNB_HTTP_SESSION_H

#include <XiaoTuNetBox/ConnectionNode.h>
#include <XiaoTuNetBox/Http/HttpRequest.h>
#include <XiaoTuNetBox/Http/HttpResponse.h>
#include <XiaoTuNetBox/Session.h>

#include <memory>
#include <string>
#include <map>
#include <typeinfo>


namespace xiaotu {
namespace net {
 
    class HttpServer;

    //! @brief 一次 Http 会话
    class HttpSession : public Session {
        public:
            enum EState {
                eExpectRequestLine,
                eReadingHeaders,
                eReadingBody,
                eResponsing,
                eError
            };
            static std::map<EState, std::string> mEStateToStringMap;

        friend class HttpServer;
        public:
            HttpSession();
            ~HttpSession();
            HttpSession(HttpSession const &) = delete;
            HttpSession & operator = (HttpSession const &) = delete;

            inline void Reset()
            {
                mRequest = std::make_shared<HttpRequest>();
                mResponse = std::make_shared<HttpResponse>();
            }

            HttpRequestPtr GetRequest() { return mRequest; }
            HttpResponsePtr GetResponse() { return mResponse; }
        
            virtual char const * ToCString() { return typeid(HttpSession).name(); }
        private:
            HttpRequestPtr mRequest;
            HttpResponsePtr mResponse;
    };

    typedef std::shared_ptr<HttpSession> HttpSessionPtr;
    typedef std::weak_ptr<HttpSession> HttpSessionWeakPtr;


}
}




#endif
