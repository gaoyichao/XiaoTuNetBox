#include <XiaoTuNetBox/WebSocketHandler.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <iostream>
#include <string>
#include <map>

#include <endian.h>
#include <glog/logging.h>

namespace xiaotu {
namespace net {

    std::map<WebSocketHandler::EState, std::string> WebSocketHandler::mEStateToStringMap = {
        { eConnecting,     "Connecting" },
        { eOpen,           "Open" },
        { eClosing,        "Closing" },
        { eClosed,         "Closed" },
        { eError,          "Error" },
    };


    WebSocketHandler::WebSocketHandler()
        : Handler()
    {
        mState = eConnecting;
    }

    WebSocketHandler::~WebSocketHandler()
    {
        LOG(INFO) << "释放会话";
    }

    bool WebSocketHandler::CheckHandShake(HttpRequestPtr const & req)
    {
        std::string value;

        if (!req->GetHeader("Upgrade", value))
            return false;
        if ("websocket" != value)
            return false;

        std::string host;
        if (!req->GetHeader("Host", host))
            return false;

        std::string secversion, seckey;
        if (!req->GetHeader("Sec-WebSocket-Version", secversion))
            return false;
        if ("13" != secversion)
            return false;

        if (!req->HasHeader("Sec-WebSocket-Key"))
            return false;

        return true;
    }

    bool WebSocketHandler::AcceptHandShake(HttpRequestPtr const & req)
    {
        assert(CheckHandShake(req));

        req->GetHeader("Sec-WebSocket-Key", mSecKey);
        uint8_t wskey[128];
        memcpy(wskey, mSecKey.data(), mSecKey.size());
        memcpy(wskey + 24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

        char accept_str[128];
        int acc_n = Sha1Base64(wskey, 24 + 36, accept_str);
        accept_str[acc_n] = 0;
        mAccKey = std::string(accept_str);

        mHandShakeRequest = req;
        mHandShakeResponse = std::make_shared<HttpResponse>();

        mHandShakeResponse->SetClosing(false);
        mHandShakeResponse->SetStatusCode(HttpResponse::e101_SwitchProtocol);
        mHandShakeResponse->SetHeader("Connection", "Upgrade");
        mHandShakeResponse->SetHeader("Upgrade", "websocket");
        mHandShakeResponse->SetHeader("Sec-WebSocket-Accept", mAccKey);

        mHandShakeResponse->LockHead(0);
        return true;
    }

}
}
