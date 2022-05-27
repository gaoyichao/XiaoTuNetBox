#include <XiaoTuNetBox/Http/HttpModuleCheckURL.h>
#include <XiaoTuNetBox/Utils.h>
#include <XiaoTuNetBox/Task.h>

#include <regex>

#include <glog/logging.h>


namespace xiaotu {
namespace net {

    bool HttpModuleCheckURL::Process(HttpHandlerWeakPtr const & handler)
    {
        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr request = h->GetRequest();
        HttpResponsePtr res = h->GetResponse();

        std::string urlpath = request->GetURLPath();
        DLOG(INFO) << "url: " << urlpath;

        std::string path = mWorkSpace + urlpath;
        res->SetClosing(!request->KeepAlive());

        if (IsDir(path)) {
            if (IsFile(path + "/index.html")) {
                urlpath += "/index.html";
            } else {
                if (mDirModule) {
                    h->mCurrTask->SetSuccessFunc(std::bind(&HttpModule::Handle,
                                mDirModule.get(), handler));
                } else {
                    res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);
                    std::string errstr("<h1>Error:503</h1>");
                    res->LockHead(errstr.size());
                    res->AppendContent(errstr);
                }
                h->WakeUp();
                return false;
            }
        }

        path = mWorkSpace + urlpath;
        DLOG(INFO) << "资源路径: " << path;

        if (!IsReadable(path) || !res->SetFile(path)) {
            if (m404Module) {
                h->mCurrTask->SetSuccessFunc(std::bind(&HttpModule::Handle,
                            m404Module.get(), handler));
            } else {
                res->SetStatusCode(HttpResponse::e404_NotFound);
                std::string errstr("<h1>Error:404</h1>");
                res->LockHead(errstr.size());
                res->AppendContent(errstr);
            }
            h->WakeUp();
            return false;
        }

        //! @todo 检查文件类型, issue #3
        int idx = GetSuffix((uint8_t*)urlpath.data(), urlpath.size(), '.');
        if (idx > 0) {
            std::string suffix = urlpath.substr(idx);
            if (".js" == suffix)
                res->SetHeader("Content-Type", "text/javascript");
            else if (".html" == suffix)
                res->SetHeader("Content-Type", "text/html");
            else if (".svg" == suffix)
                res->SetHeader("Content-Type", "image/svg+xml");
        }

        struct stat const & s = res->GetFileStat();
        res->SetStatusCode(HttpResponse::e200_OK);

        h->WakeUp();
        return true;
    }


}
}
