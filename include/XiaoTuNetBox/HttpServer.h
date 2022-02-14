/************************************************************************************
 * 
 * HttpServer - Http 的实现
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SERVER_H
#define XTNB_HTTP_SERVER_H

#include <XiaoTuNetBox/TcpAppServer.h>
#include <XiaoTuNetBox/HttpSession.h>

#include <vector>

namespace xiaotu {
namespace net {
 
    class HttpServer : public TcpAppServer {
        public:
            HttpServer(PollLoopPtr const & loop, int port, int max_conn);

        private:
            virtual SessionPtr OnNewConnection(ConnectionPtr const & conn);
            virtual void OnCloseConnection(ConnectionPtr const & conn, SessionPtr const & session);
            virtual void OnMessage(ConnectionPtr const & con, SessionPtr const & session);

        private:
            void HandleRequest(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr);
            void HandleReponse(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr);

            void OnGetRequest(HttpRequestPtr const & req, HttpResponsePtr const & res);

    };

}
}

#endif
