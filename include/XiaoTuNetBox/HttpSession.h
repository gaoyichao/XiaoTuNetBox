/************************************************************************************
 * 
 * HttpSession - 一次Http会话
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SESSION_H
#define XTNB_HTTP_SESSION_H

#include <XiaoTuNetBox/ConnectionNode.h>
#include <XiaoTuNetBox/HttpRequest.h>
#include <XiaoTuNetBox/Session.h>

#include <memory>
#include <string>
#include <map>
#include <typeinfo>


namespace xiaotu {
namespace net {
 
    class HttpServer;
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
            HttpSession(ConnectionPtr const & conn);
            HttpSession(HttpSession const &) = delete;
            HttpSession & operator = (HttpSession const &) = delete;

            bool ParseRequestLine(uint8_t const * begin, uint8_t const * end);
            HttpRequestPtr HandleMsg(ConnectionPtr const & conn);
        
            virtual char const * ToCString() { return typeid(*this).name(); }

        private:
            void OnExpectRequestLine(ConnectionPtr const & conn);
            void OnReadingHeaders(ConnectionPtr const & conn);
            void OnReadingBody(ConnectionPtr const & conn);

        private:
            //! @todo 增加输出缓存
            InBufObserverPtr mInBuf;
            size_t mIdx;

            EState mState;
            HttpRequestPtr mRequest;
    };

    typedef std::shared_ptr<HttpSession> HttpSessionPtr;


}
}




#endif
