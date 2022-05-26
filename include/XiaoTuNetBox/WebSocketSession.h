/************************************************************************************
 * 
 * WebSocketSession - WebSocket 的一次会话
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
    class WebSocketSession : public Session {
        public:
            enum EState {
                eConnecting = 0,
                eOpen = 1,
                eClosing = 2,
                eClosed = 3,

                eReadingLen16 = 4,
                eReadingLen64 = 5,
                eReadingMask = 6,
                eReadingPayload = 7,

                eNewMsg = 8,

                eError
            };
            static std::map<EState, std::string> mEStateToStringMap;

            friend class WebSocketServer;
        public:
            WebSocketSession();
            ~WebSocketSession();
            WebSocketSession(WebSocketSession const &) = delete;
            WebSocketSession & operator = (WebSocketSession const &) = delete;

            inline std::string const & GetStateStr() const
            {
                auto it = mEStateToStringMap.find(mState);
                if (mEStateToStringMap.end() == it)
                    return mEStateToStringMap[eError];
                return it->second;
            }

            void Reset()
            {
                mRcvdMsg = std::make_shared<WebSocketMsg>();
                mState = eOpen;
            }

            bool InOpenState() const
            {
                return (mState == eOpen ||
                        mState == eReadingLen16 ||
                        mState == eReadingLen64 ||
                        mState == eReadingMask ||
                        mState == eReadingPayload ||
                        mState == eNewMsg);
            }
            EState GetState() const { return mState; }
            virtual char const * ToCString() { return typeid(WebSocketSession).name(); }

            static bool CheckHandShake(HttpRequestPtr const & req);
            bool AcceptHandShake(HttpRequestPtr const & req);
            HttpRequestPtr GetHandShakeRequest() { return mHandShakeRequest; }
            HttpResponsePtr GetHandShakeResponse() { return mHandShakeResponse; }

            WebSocketMsgPtr GetRcvdMsg() { return mRcvdMsg; }

        private:
            uint8_t const * HandleMsg        (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnOpen           (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingLen16   (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingLen64   (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingMask    (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingPayload (uint8_t const * begin, uint8_t const * end);

            uint8_t const * GetData(int n, uint8_t const * & begin, uint8_t const * & end);
        private:
            EState mState;
            HttpRequestPtr mHandShakeRequest;
            HttpResponsePtr mHandShakeResponse;
            WebSocketMsgPtr mRcvdMsg;

            std::vector<uint8_t> mReadingData;
            std::string mSecKey;
            std::string mAccKey;

        public:
            RawMsgPtr PopSendMsg()
            {
                RawMsgPtr re = nullptr;
                {
                    std::unique_lock<std::mutex> lock(mSendMsgsMutex);
                    if (!mSendMsgs.empty()) {
                        re = mSendMsgs.front();
                        mSendMsgs.pop_front();
                    }
                }
                return re;
            }

            void SendRawMsg(RawMsgPtr const & msg)
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
            std::deque<RawMsgPtr> mSendMsgs;
    };

    typedef std::shared_ptr<WebSocketSession> WebSocketSessionPtr;
    typedef std::weak_ptr<WebSocketSession> WebSocketSessionWeakPtr;

}
}




#endif
