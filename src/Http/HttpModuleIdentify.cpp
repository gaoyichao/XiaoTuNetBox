#include <XiaoTuNetBox/Http/HttpModuleIdentify.h>
#include <XiaoTuNetBox/Utils.h>
#include <XiaoTuNetBox/Task.h>

#include <regex>

#include <glog/logging.h>


namespace xiaotu {
namespace net {


    bool HttpModuleBasicIdentify::Process(HttpHandlerWeakPtr const & handler)
    {
        DLOG(INFO) << "验证身份";

        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr request = h->GetRequest();
        HttpResponsePtr res = h->GetResponse();

        if (CheckId(*request)) {
            h->WakeUp();
            return true;
        }


        res->SetStatusCode(HttpResponse::e401_Unauthorized);
        res->SetHeader("WWW-authenticate", "Basic realm=\"ni shi shui\"");
        res->LockHead(0);

        h->WakeUp();
        return false;
    }

    bool HttpModuleBasicIdentify::CheckId(HttpRequest const & request)
    {
        bool re = false;

        std::string auth;
        if (!request.GetHeader("Authorization", auth))
            return false;

        std::regex e("(Basic)\\s+([a-zA-z0-9=+/]+)");
        std::smatch m;
        if (!std::regex_search(auth, m, e))
            return false;

        //! @todo 用户管理
        std::string userpass = Base64Decode(m[2]);
        if ("douniwan:bienao" == userpass)
            return true;

        return false;
    }
}
}

