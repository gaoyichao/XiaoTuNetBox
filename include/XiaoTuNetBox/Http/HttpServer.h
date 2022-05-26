/************************************************************************************
 * 
 * HttpServer - Http 的实现
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SERVER_H
#define XTNB_HTTP_SERVER_H

#include <XiaoTuNetBox/TcpAppServer.h>
#include <XiaoTuNetBox/Http/HttpSession.h>

#include <vector>

namespace xiaotu {
namespace net {
 
    class HttpServer : public TcpAppServer {
            typedef std::function<void(ConnectionPtr const &, HttpSessionPtr const &)> ConSessionFunc;
        public:
            static const int mDefaultLoadSize;
        public:
            HttpServer(EventLoopPtr const & loop, int port, int max_conn);

        private:
            virtual void OnNewConnection(ConnectionPtr const & conn);
            virtual void OnCloseConnection(ConnectionPtr const & conn);
            virtual void OnMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n);

        public:
            static void OnTaskFinished(ConnectionWeakPtr const & conptr);
            static bool OnTaskSuccessDefault(ConnectionPtr const &, HttpSessionPtr const &);
            static bool OnTaskFailureDefault(ConnectionPtr const &, HttpSessionPtr const &);
            static bool OnTaskSuccessGet(ConnectionWeakPtr const & conptr, ThreadWorkerPtr const & worker);

            static bool HandleRequest(ConnectionPtr const & con, std::string workspace, ThreadWorkerPtr const & worker);
            static bool HandleHeadRequest      (HttpSessionWeakPtr const & weakptr);
            static bool HandleGetRequest       (HttpSessionWeakPtr const & weakptr);
            static bool HandleInvalidRequest   (HttpSessionWeakPtr const & weakptr);
            static bool HandleUnSupportRequest (HttpSessionWeakPtr const & weakptr);

            static bool HandleGetLoadContent   (HttpSessionWeakPtr const & weakptr);
            static bool HandleUnauthorized    (HttpSessionWeakPtr const & weakptr);

            static void OnHeadRequest(HttpRequestPtr const & req, HttpResponsePtr const & res);

        public:
            void SetUpgradeSessionCallBk(ConSessionFunc func) { mUpgradeSessionCallBk = std::move(func); }
        private:
            ConSessionFunc mUpgradeSessionCallBk;
    };

}
}

#endif
