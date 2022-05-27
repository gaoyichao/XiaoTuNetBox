/************************************************************************************
 * 
 * HttpHandler - 一次Http会话
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SESSION_H
#define XTNB_HTTP_SESSION_H

#include <XiaoTuNetBox/ConnectionNode.h>
#include <XiaoTuNetBox/Http/HttpRequest.h>
#include <XiaoTuNetBox/Http/HttpResponse.h>
#include <XiaoTuNetBox/Handler.h>

#include <memory>
#include <string>
#include <map>
#include <typeinfo>


namespace xiaotu {
namespace net {
 
    class HttpServer;

    //! @brief 一次 Http 会话
    class HttpHandler : public Handler {
        public:
            enum EState {
                eWaitingRequest,
            };
            static std::map<EState, std::string> mEStateToStringMap;

        friend class HttpServer;
        public:
            HttpHandler(ConnectionPtr const & con);
            ~HttpHandler();
            HttpHandler(HttpHandler const &) = delete;
            HttpHandler & operator = (HttpHandler const &) = delete;

            inline void Reset()
            {
                mRequest = std::make_shared<HttpRequest>();
                mResponse = std::make_shared<HttpResponse>();
                mState = eWaitingRequest;
            }

            ConnectionPtr GetConnection() { return mConnWeakPtr.lock(); }

            HttpRequestPtr GetRequest() { return mRequest; }
            HttpResponsePtr GetResponse() { return mResponse; }
            EState GetState() { return mState; }
            std::string GetStateStr() { return mEStateToStringMap[mState]; }
        
            virtual char const * ToCString() { return typeid(HttpHandler).name(); }
        private:
            EState mState;
            HttpRequestPtr mRequest;
            HttpResponsePtr mResponse;
        public:
            ConnectionWeakPtr mConnWeakPtr;
    };

    typedef std::shared_ptr<HttpHandler> HttpHandlerPtr;
    typedef std::weak_ptr<HttpHandler> HttpHandlerWeakPtr;


}
}




#endif
