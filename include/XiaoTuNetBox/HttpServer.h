/************************************************************************************
 * 
 * HttpServer - Http 的实现
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SERVER_H
#define XTNB_HTTP_SERVER_H

#include <XiaoTuNetBox/TcpServer.h>
#include <XiaoTuNetBox/HttpSession.h>
#include <XiaoTuNetBox/HttpRequest.h>
#include <XiaoTuNetBox/HttpResponse.h>


#include <vector>

namespace xiaotu {
namespace net {
 
    class HttpServer {
        public:
            HttpServer(PollLoopPtr const & loop, int port, int max_conn);

        private:
            SessionPtr OnNewConnection(ConnectionPtr const & conn);
            void OnCloseConnection(ConnectionPtr const & conn, SessionPtr const & session);
            void OnMessage(ConnectionPtr const & con, SessionPtr const & session);
            void HandleRequest(ConnectionPtr const & con, HttpSessionPtr const & session);
    
        public:
            typedef std::function< void (HttpRequestPtr const &, HttpResponsePtr const &)> RequestCallBk;
            void SetRequestCallBk(RequestCallBk cb) { mRequestCB = std::move(cb); }

        private:
            RequestCallBk mRequestCB;

        private:
            std::vector<HttpSessionPtr> mSessions;
            std::vector<size_t> mHoles;
            TcpServer mServer;

    };

}
}

#endif
