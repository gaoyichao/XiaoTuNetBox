/************************************************************************************
 * 
 * HttpRequest 
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_RESPONSE_H
#define XTNB_HTTP_RESPONSE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

namespace xiaotu {
namespace net {

    class HttpResponse {
        public:
            enum EStatusCode {
                eUnknown = 0,
                e200_OK = 200,
                e400_BadRequest = 400,
                e404_NotFound = 404,
                e503_ServiceUnavilable = 503
            };
            static std::map<EStatusCode, std::string> mEStatusCodeToStringMap;
        public:
            HttpResponse(bool close = true)
                : mStatusCode(eUnknown), mStatusMessage(""), mCloseConnection(close)
            { }

            void SetStatusCode(EStatusCode code)
            {
                mStatusCode = code;
                mStatusMessage = GetStatusCodeStr();
            }

            EStatusCode GetStatusCode() const { return mStatusCode; }
            inline std::string const & GetStatusCodeStr() const
            {
                auto it = mEStatusCodeToStringMap.find(mStatusCode);
                if (mEStatusCodeToStringMap.end() == it)
                    return mEStatusCodeToStringMap[eUnknown];
                return it->second;
            }

            void SetStatusMessage(std::string const & msg) { mStatusMessage = msg; }
            std::string const & GetStatusMessage() const { return mStatusMessage; }

            bool CloseConnection()
            {
                return mCloseConnection || (e200_OK != mStatusCode);
            }

        public:
            void StartLine(std::string & sl);
            void ToUint8Vector(std::vector<uint8_t> & buf);

            void AppendContent(std::string const & content)
            {
                mContent.insert(mContent.end(), content.begin(), content.end());
            }

            void AppendContent(std::vector<uint8_t> const & content)
            {
                mContent.insert(mContent.end(), content.begin(), content.end());
            }

            std::map<std::string, std::string> const & GetHeader() const { return mHeaders; }
        private:
            std::map<std::string, std::string> mHeaders;
            std::vector<uint8_t> mContent;

            EStatusCode mStatusCode;
            std::string mStatusMessage;
            bool mCloseConnection;
    };
    typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

}
}

#endif
