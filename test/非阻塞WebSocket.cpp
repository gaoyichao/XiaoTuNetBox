/******************************************************************************
 * 
 * 非阻塞echo_polloop - echo服务器例程
 * 
 * 单线程
 * 
 * 对应运行 t_stdin_ipv4_talk
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/WebSocketServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <functional>
#include <iostream>
#include <string>
#include <map>

#include <endian.h>
#include <string.h>

using namespace std::placeholders;
using namespace xiaotu::net;

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
}

int main() {
    PollLoopPtr loop = CreatePollLoop();
    WebSocketServer tcp(loop, 65530, 3);

    tcp.SetTimeOut(10, 0, 5);
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));

    loop->Loop(1000);

    return 0;
}

