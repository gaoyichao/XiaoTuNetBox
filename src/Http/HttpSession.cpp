/************************************************************************************
 * 
 * HttpSession - 一次Http会话
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/Http/HttpSession.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <iostream>
#include <string>
#include <map>

#include <glog/logging.h>

namespace xiaotu {
namespace net {

    std::map<HttpSession::EState, std::string> HttpSession::mEStateToStringMap = {
        { eExpectRequestLine, "Expect Request Line" },
        { eReadingHeaders,    "Reading Headers" },
        { eReadingBody,       "Reading Body" },
        { eResponsing,        "Responsing" },
        { eError,             "Error" },
    };

    HttpSession::HttpSession()
    {
        Reset();
    }

    HttpSession::~HttpSession()
    {
        LOG(INFO) << "释放会话";
    }

}
}
