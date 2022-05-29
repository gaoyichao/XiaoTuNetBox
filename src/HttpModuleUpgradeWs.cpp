#include <XiaoTuNetBox/HttpModuleUpgradeWs.h>
#include <XiaoTuNetBox/Utils.h>
#include <XiaoTuNetBox/Task.h>
#include <XiaoTuNetBox/WebSocketHandler.h>
#include <XiaoTuNetBox/WebSocketServer.h>

#include <regex>

#include <glog/logging.h>


namespace xiaotu {
namespace net {

    bool HttpModuleUpgradeWs::Process(HttpHandlerWeakPtr const & handler)
    {
        DLOG(INFO) << "升级 websocket";

        HttpHandlerPtr h = handler.lock();
        if (nullptr == h)
            return false;

        HttpRequestPtr request = h->GetRequest();

        if (!request->NeedUpgrade()) {
            h->WakeUp();
            return false;
        }

        ConnectionPtr con = h->GetConnection();
        if (nullptr == con)
            return false;

        if (!WebSocketHandler::CheckHandShake(request)) {
            if (mHandShakeFailed) {
                h->mCurrTask->SetSuccessFunc(std::bind(&HttpModule::Handle,
                            mHandShakeFailed.get(), handler));
            }
            h->WakeUp();
            return false;
        }

        WebSocketHandlerPtr ptr(new WebSocketHandler);
        ptr->AcceptHandShake(request);
        con->mUserObject = ptr;

        HttpResponsePtr res = ptr->GetHandShakeResponse();
        std::vector<uint8_t> const & buf = res->GetContent();
        con->SendBytes(buf.data(), buf.size());
        
        ptr->Reset();
        mWs->ReplaceHandler(h, ptr);
        ptr->mWakeUpper->SetWakeUpCallBk(std::bind(&WebSocketServer::HandleWsReponse,
                    ConnectionWeakPtr(con)));

        if (mWs->mNewHandlerCallBk)
            mWs->mNewHandlerCallBk(ptr);

        ptr->WakeUp();
        return true;
    }
}
}
