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
        public:
            HttpRequest()
                : mMethod(eINVALID)
            { }

            bool SetMethod(std::string const & str);
            EMethod GetMethod() const { return mMethod; }
            inline std::string const & GetMethodStr() const
            {
                auto it = mEMethodToStringMap.find(mMethod);
                if (mEMethodToStringMap.end() == it)
                    return mEMethodToStringMap[eINVALID];
                return it->second;
            }

            std::string const & SetURL(std::string const & url)
            {
                mURL = url;
                return mURL;
            }
            std::string const & GetURL() const { return mURL; }

            std::string const & SetURLPath(std::string const & path)
            {
                mURLPath = path;
                return mURLPath;
            }
            std::string const & GetURLPath() const { return mURLPath; }

            std::string const & SetURLQuery(std::string const & q)
            {
                mURLQuery = q;
                return mURLQuery;
            }
            std::string const & GetURLQuery() const { return mURLQuery; }

            std::string const & SetVersion(std::string const & ver)
            {
                mVersion = ver;
                return mVersion;
            }
            std::string const & GetVersion() const { return mVersion; }


            void SetHeader(std::string const & key, std::string const & value)
            {
                mHeaders[key] = value;
            }

            size_t ContentLength()
            {
                switch (mMethod) {
                    case eGET:
                    case eHEAD:
                    case eDELETE:
                    case eINVALID:
                        return 0;
                    default: {
                        auto it = mHeaders.find("Content-Length");
                        if (mHeaders.end() != it)
                            return std::stoul(it->second);
                        return 0;
                    }
                }
            }

            std::map<std::string, std::string> const & GetHeader() const { return mHeaders; }

            bool GetHeader(std::string const & key, std::string & value) const
            {
                auto it = mHeaders.find(key);
                if (mHeaders.end() != it) {
                    value = it->second;
                    return true;
                }
                return false;
            }

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
            std::map<std::string, std::string> mHeaders;
            std::vector<uint8_t> mContent;
    };
    typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

}
}
 
#endif
