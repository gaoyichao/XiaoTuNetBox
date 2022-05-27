#include <XiaoTuNetBox/Http/HttpModule.h>
#include <XiaoTuNetBox/Http/HttpServer.h>
#include <XiaoTuNetBox/Task.h>
#include <XiaoTuNetBox/Utils.h>

#include <glog/logging.h>

namespace xiaotu {
namespace net {


    bool HttpModule::Handle(HttpHandlerWeakPtr const & handler)
    {
        HttpHandlerPtr ptr = std::static_pointer_cast<HttpHandler>(handler.lock());

        TaskPtr task = std::make_shared<Task>(std::bind(&HttpModule::Process, this, handler));
        if (mSuccessModule)
            task->SetSuccessFunc(std::bind(&HttpModule::Handle, mSuccessModule.get(), HttpHandlerWeakPtr(ptr)));
        if (mFailureModule)
            task->SetFailureFunc(std::bind(&HttpModule::Handle, mFailureModule.get(), HttpHandlerWeakPtr(ptr)));
        ptr->mCurrTask = task;

        if (mImmediately) {
            task->Finish();
        }

        return true;
    }

    //! @brief 检查并处理非法请求
    bool HttpModuleInvalidRequest::Process(HttpHandlerWeakPtr const & handler)
    {
        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr req = h->GetRequest();
        if (HttpRequest::eINVALID == req->GetMethod()) {
            DLOG(INFO) << "非法请求";

            HttpResponsePtr res = h->GetResponse();
            res->SetStatusCode(HttpResponse::e400_BadRequest);
            res->LockHead(0);

            h->WakeUp();
            return false;
        }

        h->WakeUp();
        return true;
    }

    //! @brief 解析请求报文中 Cookie
    bool HttpModuleParseCookie::Process(HttpHandlerWeakPtr const & handler)
    {
        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr request = h->GetRequest();

        request->PrintHeaders();

        std::string cookiestr;
        if (request->GetHeader("Cookie", cookiestr)) {
            uint8_t const * begin = (uint8_t const *)cookiestr.data();
            uint8_t const * end = begin + cookiestr.size();

            while (begin <= end) {
                uint8_t const * eq = FindString(begin, end, (uint8_t const *)"=", 1);
                uint8_t const * sp = FindString(eq, end, (uint8_t const *)";", 1);
                if (NULL == sp)
                    sp = end;

                uint8_t const * key_begin = EatByte(begin, eq, ' ');
                uint8_t const * key_end = eq;
                while (' ' == key_end[-1]) key_end--;
 
                uint8_t const * val_begin = eq + 1;
                while (*val_begin == ' ') val_begin++;
                uint8_t const * val_end = sp;
                while (' ' == val_end[-1]) val_end--;

                uint32_t key_len = key_end - key_begin;
                uint32_t val_len = val_end - val_begin;
     
                std::string key((char const *)key_begin, key_len);
                std::string value((char const *)val_begin, val_len);
                request->mCookies[key] = value;

                std::cout << key << "=" << value << std::endl;
                begin = sp + 1;
            }
        }

        h->WakeUp();
        return true;
    }

    //! @brief 发送响应报文
    bool HttpModuleResponse::Process(HttpHandlerWeakPtr const & handler)
    {
        DLOG(INFO) << "发送响应报文";

        HttpHandlerPtr h = handler.lock();
        ConnectionPtr con = h->GetConnection();
        if (nullptr == con || con->IsClosed()) {
            h->Reset();
            return false;
        }

        HttpResponsePtr res = h->GetResponse();
        std::vector<uint8_t> const & buf = res->GetContent();
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        h->Reset();
        return true;
    }

}
}


