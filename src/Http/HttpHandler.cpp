/************************************************************************************
 * 
 * HttpHandler - 一次Http会话
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/Http/HttpHandler.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <iostream>
#include <string>
#include <map>

#include <glog/logging.h>

namespace xiaotu {
namespace net {

    std::map<HttpHandler::EState, std::string> HttpHandler::mEStateToStringMap = {
        { eWaitingRequest, "等待请求报文" },
    };

    HttpHandler::HttpHandler(ConnectionPtr const & con)
        : mConnWeakPtr(con)
    {
        Reset();
    }

    HttpHandler::~HttpHandler()
    {
        LOG(INFO) << "释放会话";
    }

}
}
