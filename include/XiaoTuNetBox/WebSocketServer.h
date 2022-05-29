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
#include <XiaoTuNetBox/WebSocketHandler.h>
#include <XiaoTuNetBox/Http/HttpHandler.h>
#include <XiaoTuNetBox/Http/HttpModule.h>


namespace xiaotu {
namespace net {
 
    class WebSocketServer : public TcpAppServer {
        public:
            WebSocketServer(EventLoopPtr const & loop, int port,
                       int max_conn, std::string const & ws);

        private:
            virtual void OnNewConnection(ConnectionPtr const & conn);
            virtual void OnCloseConnection(ConnectionPtr const & conn);
            virtual void OnMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n);

        private:
            void OnHttpMessage(ConnectionPtr const & conn, uint8_t const * buf, ssize_t n);
            void OnWsMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n);

        public:
            static void HandleWsReponse(ConnectionWeakPtr const & conptr);

            typedef std::function<void(WebSocketHandlerPtr const & session)> HandlerCallBk;
            typedef std::function<void(WebSocketHandlerPtr const & session, WebSocketMsgPtr const & msg)> MsgCallBk;

            void SetMsgCallBk(MsgCallBk cb) { mMsgCallBk = std::move(cb); }
            void SetNewHandlerCallBk(HandlerCallBk cb) { mNewHandlerCallBk = std::move(cb); }

            HandlerCallBk mNewHandlerCallBk;
            MsgCallBk mMsgCallBk;
        private:

            HttpModulePtr mFirstModule;
    };

    typedef std::shared_ptr<WebSocketServer> WebSocketServerPtr;
    typedef std::weak_ptr<WebSocketServer> WebSocketServerWeakPtr;


}
}

#endif
