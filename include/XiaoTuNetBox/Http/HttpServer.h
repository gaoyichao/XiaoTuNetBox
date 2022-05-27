/************************************************************************************
 * 
 * HttpServer - Http 的实现
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_SERVER_H
#define XTNB_HTTP_SERVER_H

#include <XiaoTuNetBox/TcpAppServer.h>
#include <XiaoTuNetBox/Http/HttpHandler.h>
#include <XiaoTuNetBox/Http/HttpModule.h>

#include <vector>
#include <memory>

namespace xiaotu {
namespace net {
 
    class HttpHandler;
    class HttpServer : public TcpAppServer {
        public:
            static const int mDefaultLoadSize;
        public:
            HttpServer(EventLoopPtr const & loop, int port,
                       int max_conn, std::string const & ws);

        private:
            virtual void OnNewConnection(ConnectionPtr const & conn);
            virtual void OnCloseConnection(ConnectionPtr const & conn);
            virtual void OnMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n);

        public:
            static void OnTaskFinished(ConnectionWeakPtr const & conptr);
            static bool OnTaskSuccessDefault(ConnectionPtr const &, HttpHandlerPtr const &);
            static bool OnTaskFailureDefault(ConnectionPtr const &, HttpHandlerPtr const &);

        private:
            HttpModulePtr mFirstModule;
    };

}
}

#endif
