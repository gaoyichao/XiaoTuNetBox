#include <XiaoTuNetBox/Http/HttpModule.h>
#include <XiaoTuNetBox/Http/HttpServer.h>
#include <XiaoTuNetBox/Task.h>

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


