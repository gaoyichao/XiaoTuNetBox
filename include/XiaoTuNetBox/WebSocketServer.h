/************************************************************************************
 * 
 * WebSocketServer - WebSocket 的实现
 * 
 * https://datatracker.ietf.org/doc/html/rfc6455
 * 
 ***********************************************************************************/
#ifndef XTNB_WEB_SOCKET_SERVER_H
#define XTNB_WEB_SOCKET_SERVER_H

#include <XiaoTuNetBox/TcpServer.h>
#include <XiaoTuNetBox/WebSocketSession.h>

#include <vector>

namespace xiaotu {
namespace net {
 
    class WebSocketServer : public TcpServer {
        public:
            WebSocketServer(PollLoopPtr const & loop, int port, int max_conn);

        private:
            void OnNewConnection(ConnectionPtr const & conn);
    
        private:
            std::vector<WebSocketSessionPtr> mSessions;

    };

}
}

#endif
