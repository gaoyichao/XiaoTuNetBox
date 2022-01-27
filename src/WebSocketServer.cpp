#include <XiaoTuNetBox/WebSocketServer.h>

#include <cassert>
#include <functional>


namespace xiaotu {
namespace net {

    using namespace std::placeholders;

    WebSocketServer::WebSocketServer(PollLoopPtr const & loop, int port, int max_conn)
        : TcpServer(loop, port, max_conn)
    {
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        SetNewConnCallBk(std::bind(&WebSocketServer::OnNewConnection, this, _1));
    }


    void WebSocketServer::OnNewConnection(ConnectionPtr const & conn) {
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        std::cout << "新建连接:" << conn->GetInfo() << std::endl;
        conn->GetHandler()->SetNonBlock(true);
        mSessions.push_back(WebSocketSessionPtr(new WebSocketSession(conn)));
    }


}
}
