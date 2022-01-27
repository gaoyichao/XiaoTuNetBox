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

#include <memory>
#include <string>
#include <map>


namespace xiaotu {
namespace net {
 
    class WebSocketSession {
        enum eState {
            eIdle = 0,
            eConnecting = 1,
            eReady = 2
        };

        public:
            WebSocketSession(ConnectionPtr const & conn);
            void HandleMsg();
        private:
            ConnectionPtr mConn;
            InBufObserverPtr mObserver;

            eState mState;
            std::string mRequestURI;
            std::map<std::string, std::string> mRequestMap;
    };

    typedef std::shared_ptr<WebSocketSession> WebSocketSessionPtr;


}
}




#endif
