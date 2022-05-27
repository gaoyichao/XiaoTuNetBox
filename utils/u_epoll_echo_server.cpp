/******************************************************************************
 * 
 * t_echo_server_0 - echo服务器例程
 * 
 * 单线程
 * 
 * 对应运行t_echo_client_0
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/Event.h>
#include <XiaoTuNetBox/TcpServer.h>

#include <memory>
#include <vector>
#include <functional>
#include <typeinfo>

using namespace std::placeholders;
using namespace xiaotu::net;

HandlerPtr OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetInfo() << std::endl;
    conn->GetHandler()->SetNonBlock(true);
    return nullptr;
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
}

void OnMessage(ConnectionPtr const & con, uint8_t const * buf, size_t n)
{
    std::cout << "接收到了消息" << std::endl;
    std::cout << "n:" << n << std::endl;

    for (unsigned int i = 0; i < n; i++)
        std::cout << buf[i];
    std::cout << std::endl;

    con->SendBytes(buf, n);
}

int main() {
    EPollLoopPtr loop = Create<EPollLoop>();
    TcpServer tcp(loop, 65530, 3);

    tcp.SetTimeOut(10, 0, 5);
    tcp.SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));
    tcp.SetMessageCallBk(std::bind(OnMessage, _1, _2, _3));

    loop->Loop(10);

    return 0;
}

