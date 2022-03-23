/************************************************************************************
 * 
 * HttpSession - 一次Http会话
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SESSION_H
#define XTNB_HTTP_SESSION_H

#include <XiaoTuNetBox/ConnectionNode.h>
#include <XiaoTuNetBox/HttpRequest.h>
#include <XiaoTuNetBox/HttpResponse.h>
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
            HttpSession(ConnectionPtr const & conn);
            ~HttpSession();
            HttpSession(HttpSession const &) = delete;
            HttpSession & operator = (HttpSession const &) = delete;

            //! @brief 判定当前是否在解析请求报文
            inline bool InRequestPhase() const
            {
                return (mState == eExpectRequestLine ||
                        mState == eReadingHeaders ||
                        mState == eReadingBody);
            }
            //! @brief 判定当前是否需要发送响应报文
            inline bool InResponsePhase() const
            {
                return eResponsing == mState;
            }

            inline std::string const & GetStateStr() const
            {
                auto it = mEStateToStringMap.find(mState);
                if (mEStateToStringMap.end() == it)
                    return mEStateToStringMap[eError];
                return it->second;
            }

            inline void Reset()
            {
                mRequest = std::make_shared<HttpRequest>();
                mResponse = std::make_shared<HttpResponse>();
                mState = eExpectRequestLine;
            }

            EState GetState() const { return mState; }

            bool ParseRequestLine(uint8_t const * begin, uint8_t const * end);
            HttpRequestPtr HandleRequest(ConnectionPtr const & conn);

            HttpRequestPtr GetRequest() { return mRequest; }
            HttpResponsePtr GetResponse() { return mResponse; }
        
            virtual char const * ToCString() { return typeid(HttpSession).name(); }

        private:
            bool OnExpectRequestLine(ConnectionPtr const & conn);
            bool OnReadingHeaders(ConnectionPtr const & conn);
            bool OnReadingBody(ConnectionPtr const & conn);

        private:
            EState mState;
            HttpRequestPtr mRequest;
            HttpResponsePtr mResponse;
    };

    typedef std::shared_ptr<HttpSession> HttpSessionPtr;
    typedef std::weak_ptr<HttpSession> HttpSessionWeakPtr;


}
}




#endif
