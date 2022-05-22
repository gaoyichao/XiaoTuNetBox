/************************************************************************************
 * 
 * WebSocketServer - WebSocket 的实现
 * 
 * https://datatracker.ietf.org/doc/html/rfc6455
 * 
 ***********************************************************************************/
#ifndef XTNB_WEB_SOCKET_SERVER_H
#define XTNB_WEB_SOCKET_SERVER_H

#include <XiaoTuNetBox/TcpAppServer.h>
#include <XiaoTuNetBox/WebSocketSession.h>
#include <XiaoTuNetBox/Http/HttpSession.h>


namespace xiaotu {
namespace net {
 
    class WebSocketServer : public TcpAppServer {
        public:
            WebSocketServer(EventLoopPtr const & loop, int port, int max_conn);

        private:
            virtual void OnNewConnection(ConnectionPtr const & conn);
            virtual void OnCloseConnection(ConnectionPtr const & conn);
            virtual void OnMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n);

        private:
            void OnHttpMessage(ConnectionPtr const & conn, uint8_t const * buf, ssize_t n);
            void OnHttpResponse(ConnectionPtr const & con, HttpSessionPtr const & session);

            void OnWsMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n);
            void OnWsConResponse(ConnectionPtr const & con, WebSocketSessionPtr const & session);
            WebSocketSessionPtr UpgradeSession(ConnectionPtr const & con, HttpSessionPtr const & session);

            void HandleMessage(WebSocketSessionWeakPtr const & weakptr, WebSocketMsgPtr const & msg);
            void HandleRequest(ConnectionWeakPtr const & conptr, HttpSessionWeakPtr const & weakptr);
            void HandleReponse(ConnectionWeakPtr const & conptr);

            void OnGetRequest(HttpRequestPtr const & req, HttpResponsePtr const & res);


        public:
            typedef std::function<void(WebSocketSessionPtr const & session)> SessionCallBk;
            typedef std::function<void(WebSocketSessionPtr const & session, WebSocketMsgPtr const & msg)> MsgCallBk;

            void SetMsgCallBk(MsgCallBk cb) { mMsgCallBk = std::move(cb); }
            void SetNewSessionCallBk(SessionCallBk cb) { mNewSessionCallBk = std::move(cb); }
        private:
            MsgCallBk mMsgCallBk;
            SessionCallBk mNewSessionCallBk;

    };

    typedef std::shared_ptr<WebSocketServer> WebSocketServerPtr;
    typedef std::weak_ptr<WebSocketServer> WebSocketServerWeakPtr;


}
}

#endif