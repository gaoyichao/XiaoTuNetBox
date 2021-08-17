/******************************************************************************
 * 
 * t_echo_server_0 - echo服务器例程
 * 
 * 单线程
 * 
 * 对应运行t_echo_client_0
 * 
 *****************************************************************************/

#include <XiaoTuNetBox/TcpServer.h>

#include <functional>

using namespace std::placeholders;
using namespace xiaotu::net;

void OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetInfo() << std::endl;
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
}

uint8_t * as = NULL;
const int aslen = 5253120;
void OnNewRawMsg(ConnectionPtr const & conn, RawMsgPtr const & msg) {
    conn->SendBytes(as, aslen);
    conn->SendRawMsg(msg);
}



int main() {
    as = (uint8_t *)malloc(aslen);
    assert(NULL != as);
    memset(as, 'A', aslen);

    PollLoopPtr loop = CreatePollLoop();
    TcpServer tcp(loop, 65530, 3);

    tcp.SetTimeOut(10, 0, 5);
    tcp.SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));
    tcp.SetNewRawMsgCallBk(std::bind(OnNewRawMsg, _1, _2));

    loop->Loop(1000);

    return 0;
}

