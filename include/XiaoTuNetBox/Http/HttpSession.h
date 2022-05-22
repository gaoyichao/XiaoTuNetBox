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

            HttpRequestPtr GetRequest() { return mRequest; }
            HttpResponsePtr GetResponse() { return mResponse; }
        
            virtual char const * ToCString() { return typeid(HttpSession).name(); }

            bool ParseRequestLine(uint8_t const * begin, uint8_t const * end);
            uint8_t const * HandleRequest(uint8_t const * begin, uint8_t const * end);
        private:
            uint8_t const * GetLine             (uint8_t const * & begin, uint8_t const * & end);
            uint8_t const * OnExpectRequestLine (uint8_t const *   begin, uint8_t const *   end);
            uint8_t const * OnReadingHeaders    (uint8_t const *   begin, uint8_t const *   end);
            uint8_t const * OnReadingBody       (uint8_t const *   begin, uint8_t const *   end);

            std::string mReadingLine;
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
