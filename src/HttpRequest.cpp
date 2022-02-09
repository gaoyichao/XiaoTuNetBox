#include <XiaoTuNetBox/HttpRequest.h>
#include <XiaoTuNetBox/Utils.h>

#include <iostream>

namespace xiaotu {
namespace net {
    std::map<std::string, HttpRequest::EMethod> HttpRequest::mStringToEMethodMap = {
        { "GET",    eGET    },
        { "POST",   ePOST   },
        { "HEAD",   eHEAD   },
        { "PUT",    ePUT    },
        { "DELETE", eDELETE },
    };

    std::map<HttpRequest::EMethod, std::string> HttpRequest::mEMethodToStringMap = {
        { eGET,     "GET"     },
        { ePOST,    "POST"    },
        { eHEAD,    "HEAD"    },
        { ePUT,     "PUT"     },
        { eDELETE,  "DELETE"  },
        { eINVALID, "INVALID" },
    };

    bool HttpRequest::SetMethod(std::string const & str)
    {
        auto it = mStringToEMethodMap.find(str);
        if (mStringToEMethodMap.end() == it)
            mMethod = eINVALID;
        else
            mMethod = it->second;
        return eINVALID != mMethod;
    }

}
}

