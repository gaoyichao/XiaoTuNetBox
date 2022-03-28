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
            typedef std::function<void(ConnectionPtr const &, HttpSessionPtr const &)> ConSessionFunc;
        public:
            HttpServer(PollLoopPtr const & loop, int port, int max_conn);
            HttpServer(TcpServerPtr const & server);

			void Init();
        private:
            virtual void OnNewConnection(ConnectionPtr const & conn);
            virtual void OnCloseConnection(ConnectionPtr const & conn);
            virtual void OnMessage(ConnectionPtr const & con);

        private:
            void HandleRequest(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr);
            void HandleReponse(ConnectionPtr const & con, HttpSessionWeakPtr const & weakptr);
            void OnGetRequest(HttpRequestPtr const & req, HttpResponsePtr const & res);

        public:
            void SetUpgradeSessionCallBk(ConSessionFunc func) { mUpgradeSessionCallBk = std::move(func); }
        private:
            ConSessionFunc mUpgradeSessionCallBk;
    };

}
}

#endif
