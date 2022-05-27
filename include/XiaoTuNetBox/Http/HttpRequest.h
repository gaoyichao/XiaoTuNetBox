/************************************************************************************
 * 
 * HttpRequest 
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_REQUEST_H
#define XTNB_HTTP_REQUEST_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

namespace xiaotu {
namespace net {

    class HttpRequest {
        public:
            //! HTTP 请求方法
            enum EMethod {
                eGET,
                ePOST,
                eHEAD,
                ePUT,
                eDELETE,
                eINVALID
            };
            static std::map<std::string, EMethod> mStringToEMethodMap;
            static std::map<EMethod, std::string> mEMethodToStringMap;

            //! HTTP 请求报文解析状态机
            enum EState {
                eExpectRequestLine,
                eReadingHeaders,
                eReadingBody,
                eResponsing,
                eError
            };
            static std::map<EState, std::string> mEStateToStringMap;

        public:
            HttpRequest()
                : mState(eExpectRequestLine), mMethod(eINVALID)
            { }

            EState GetState() const { return mState; }
            inline std::string const & GetStateStr() const
            {
                return mEStateToStringMap[mState];
            }

            EMethod GetMethod() const { return mMethod; }
            inline std::string const & GetMethodStr() const
            {
                return mEMethodToStringMap[mMethod];
            }

            std::string const & GetURL() const { return mURL; }
            std::string const & GetURLPath() const { return mURLPath; }
            std::string const & GetURLQuery() const { return mURLQuery; }
            std::string const & GetVersion() const { return mVersion; }

        public:
            uint8_t const * Parse(uint8_t const * begin, uint8_t const * end);
            bool ParseRequestLine(uint8_t const * begin, uint8_t const * end);

        private:
            uint8_t const * GetLine             (uint8_t const * & begin, uint8_t const * & end);
            uint8_t const * OnExpectRequestLine (uint8_t const *   begin, uint8_t const *   end);
            uint8_t const * OnReadingHeaders    (uint8_t const *   begin, uint8_t const *   end);
            uint8_t const * OnReadingBody       (uint8_t const *   begin, uint8_t const *   end);

            void SetMethod(EMethod method) { mMethod = method; }
            bool SetMethod   (std::string const & str);
            void SetURL      (std::string const & str) { mURL = str; }
            void SetURLPath  (std::string const & str) { mURLPath = str; }
            void SetURLQuery (std::string const & str) { mURLQuery = str; }
            void SetVersion  (std::string const & str) { mVersion = str; }
            void SetHeader(std::string const & key, std::string const & value)
            {
                mHeaders[key] = value;
            }

        private:
            EState mState;
            std::string mReadingLine;

        public:
            size_t ContentLength();

            bool HasHeader(std::string const & key) const
            {
                return (mHeaders.end() != mHeaders.find(key));
            }

            bool GetHeader(std::string const & key, std::string & value) const
            {
                auto it = mHeaders.find(key);
                if (mHeaders.end() != it) {
                    value = it->second;
                    return true;
                }
                return false;
            }

            bool GetCookie(std::string const & key, std::string & value) const
            {
                auto it = mCookies.find(key);
                if (mCookies.end() != it) {
                    value = it->second;
                    return true;
                }
                return false;
            }

            bool KeepAlive() const;
            bool NeedUpgrade() const;

            void PrintHeaders() const 
            {
                std::cout << "--------------------不要闹" << std::endl;
                for (auto it = mHeaders.begin(); it != mHeaders.end(); ++it)                
                    std::cout << it->first << ":" << it->second << std::endl;;
                std::cout << "--------------------好的" << std::endl;
            }

        public:
            EMethod mMethod;
            std::string mURL;
            std::string mURLPath;
            std::string mURLQuery;
            std::string mVersion;
            std::string mWorkSpace;
            std::map<std::string, std::string> mHeaders;
            std::vector<uint8_t> mContent;

            std::map<std::string, std::string> mCookies;
    };
    typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

}
}
 
#endif
