#include <XiaoTuNetBox/Http/HttpModuleLoadFile.h>
#include <XiaoTuNetBox/Utils.h>
#include <XiaoTuNetBox/Task.h>

#include <regex>
#include <functional>

#include <glog/logging.h>


namespace xiaotu {
namespace net {
    const int HttpModuleLoadFile::mDefaultLoadSize = 8192;

    bool HttpModuleLoadFile::Process(HttpHandlerWeakPtr const & handler)
    {
        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr req = h->GetRequest();
        HttpResponsePtr res = h->GetResponse();

        struct stat const & s = res->GetFileStat();

        res->LockHead(s.st_size);
        res->LoadContent(mDefaultLoadSize);
        h->mCurrTask->SetSuccessFunc(std::bind(&HttpModuleLoadFile::NextLoad, this, h->mConnWeakPtr));
        h->WakeUp();
        return true;
    }

    bool HttpModuleLoadFile::NextLoad(ConnectionWeakPtr const & conptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return false;
        HttpHandlerPtr h = std::static_pointer_cast<HttpHandler>(con->mUserObject.lock());
        if (nullptr == h)
            return false;

        HttpResponsePtr res = h->GetResponse();

        int endidx = res->GetHeadEndIdx();
        int loadCount = res->GetLoadCount();
        size_t nleft = (0 == loadCount) ? 0 : res->GetDataSize();
        std::vector<uint8_t> const & buf = res->GetContent();

        if (loadCount <= 1) {
            con->SendBytes(buf.data(), buf.size());
        } else {
            con->SendBytes(buf.data() + endidx, buf.size() - endidx);
        }

        if (nleft > 0) {
            res->LoadContent(mDefaultLoadSize);
            h->WakeUp();
        } else {
            if (mSuccessModule) {
                h->mCurrTask->SetSuccessFunc(std::bind(&HttpModule::Handle, mSuccessModule.get(), HttpHandlerWeakPtr(h)));
                h->WakeUp();
            } else {
                if (res->CloseConnection()) {
                    con->Close();
                }
                h->Reset();
            }
        }
        return true;
    }

}
}
