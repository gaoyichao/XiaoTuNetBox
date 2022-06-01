#include <XiaoTuNetBox/Http/HttpModuleLogin.h>
#include <XiaoTuNetBox/Utils.h>
#include <XiaoTuNetBox/Task.h>

#include <regex>

#include <glog/logging.h>


namespace xiaotu {
namespace net {

    bool HttpModuleLogin::Process(HttpHandlerWeakPtr const & handler)
    {
        DLOG(INFO) << "登录认证";

        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr request = h->GetRequest();
        HttpResponsePtr response = h->GetResponse();

        std::string host;
        if (!request->GetHeader("Host", host))
            return FailureReturn(h, response);

        std::string loginpath = "http://" + host + mLoginHtml;
        std::string urlpath = "http://" + host + request->GetURLPath();
        HttpRequest::EMethod method = request->GetMethod();

        if (urlpath == loginpath) {
            if (HttpRequest::eGET == method) {
                DLOG(INFO) << "加载登录界面";
                mRefererList.clear();
                h->WakeUp();
                return true;
            } else {
                DLOG(INFO) << "非法请求";
                return FailureReturn(h, response);
            }
        } else {
            if (HttpRequest::ePOST == method) {
                std::string str((const char *)request->mContent.data(), request->mContent.size());
                std::regex e("(username=(.*))&(passwd=(.*))");
                std::smatch m;
                if (!std::regex_search(str, m, e))
                    return FailureReturn(h, response);

                if ("douniwan" == m[2] && "bienao" == m[4]) {
                    response->SetHeader("Set-cookie", "id=douniwan");
                    h->WakeUp();
                    return true;
                }
                return FailureReturn(h, response);
            }

            std::string referer;
            if (!request->GetHeader("Referer", referer)) {
                response->SetStatusCode(HttpResponse::e307_Redirect);
                response->SetHeader("Location", loginpath);
                std::string tmpstr("use login html instead");
                response->LockHead(tmpstr.size());
                response->AppendContent(tmpstr);
                h->WakeUp();
                return false;
            } else {
                DLOG(INFO) << "Referer: " << referer;
                if (referer != loginpath) {
                    auto it = mRefererList.find(referer);
                    if (mRefererList.end() == it) {
                        DLOG(INFO) << "源头不对";
                        return FailureReturn(h, response);
                    }
                }
             
                int idx = GetSuffix((uint8_t*)urlpath.data(), urlpath.size(), '.');
                std::string suffix = urlpath.substr(idx);
                if (".css" == suffix) {
                    mRefererList.insert("http://" + host + request->GetURLPath());
                    DLOG(INFO) << "referer list size:" << mRefererList.size();
                }

                h->WakeUp();
                return true;
            }
        }

        return FailureReturn(h, response);
    }

    bool HttpModuleLogin::FailureReturn(HttpHandlerPtr const &h, HttpResponsePtr const & response)
    {
        response->SetStatusCode(HttpResponse::e401_Unauthorized);
        std::string errstr("<h1>Unauthorized:401</h1>");
        response->LockHead(errstr.size());
        response->AppendContent(errstr);
        h->WakeUp();
        return false;

    }

}
}

