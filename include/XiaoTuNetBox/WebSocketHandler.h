/************************************************************************************
 * 
 * WebSocketHandler - WebSocket 的一次会话
 * 
 * https://datatracker.ietf.org/doc/html/rfc6455
 * 
 ***********************************************************************************/
#ifndef XTNB_WEB_SOCKET_SESSION_H
#define XTNB_WEB_SOCKET_SESSION_H

#include <XiaoTuNetBox/ConnectionNode.h>
#include <XiaoTuNetBox/Http/HttpHandler.h>
#include <XiaoTuNetBox/WebSocketMsg.h>
#include <XiaoTuNetBox/Types.h>

#include <memory>
#include <string>
#include <map>
#include <typeinfo>
#include <deque>
#include <mutex>
#include <cassert>


namespace xiaotu {
namespace net {
 
    class WebSocketServer;
    class WebSocketHandler : public Handler {
        public:
            enum EState {
                eConnecting = 0,
                eOpen = 1,
                eClosing = 2,
                eClosed = 3,
                eNewMsg = 8,

                eError
            };
            static std::map<EState, std::string> mEStateToStringMap;

            friend class WebSocketServer;
        public:
            WebSocketHandler();
            ~WebSocketHandler();
            WebSocketHandler(WebSocketHandler const &) = delete;
            WebSocketHandler & operator = (WebSocketHandler const &) = delete;

            EState GetState() const { return mState; }
            inline std::string const & GetStateStr() const
            {
                return mEStateToStringMap[mState];
            }

            void Reset()
            {
                mRcvdMsg = std::make_shared<WebSocketMsg>();
                mState = eOpen;
            }

            bool InOpenState() const
            {
                return (mState == eOpen ||
                        mState == eNewMsg);
            }

            virtual char const * ToCString() { return typeid(WebSocketHandler).name(); }

            static bool CheckHandShake(HttpRequestPtr const & req);
            bool AcceptHandShake(HttpRequestPtr const & req);

            HttpRequestPtr GetHandShakeRequest() { return mHandShakeRequest; }
            HttpResponsePtr GetHandShakeResponse() { return mHandShakeResponse; }
            WebSocketMsgPtr GetRcvdMsg() { return mRcvdMsg; }

        private:
            HttpRequestPtr mHandShakeRequest;
            HttpResponsePtr mHandShakeResponse;
            WebSocketMsgPtr mRcvdMsg;

            std::string mSecKey;
            std::string mAccKey;
            EState mState;

        public:
            WebSocketMsgPtr PopSendMsg()
            {
                WebSocketMsgPtr re = nullptr;
                {
                    std::unique_lock<std::mutex> lock(mSendMsgsMutex);
                    if (!mSendMsgs.empty()) {
                        re = mSendMsgs.front();
                        mSendMsgs.pop_front();
                    }
                }
                return re;
            }


            void SendMsg(WebSocketMsgPtr const & msg)
            {
                assert(nullptr != msg);
                {
                    std::unique_lock<std::mutex> lock(mSendMsgsMutex);
                    mSendMsgs.push_back(msg);
                }
                WakeUp();
            }

        private:
            std::mutex mSendMsgsMutex;
            std::deque<WebSocketMsgPtr> mSendMsgs;
    };

    typedef std::shared_ptr<WebSocketHandler> WebSocketHandlerPtr;
    typedef std::weak_ptr<WebSocketHandler> WebSocketHandlerWeakPtr;

}
}




#endif
