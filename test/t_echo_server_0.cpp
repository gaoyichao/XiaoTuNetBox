#include <XiaoTuNetBox/TcpServer.h>

#include <functional>

using namespace std::placeholders;
using namespace xiaotu::net;

void OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetPeerAddr().GetIpPort() << std::endl;
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetPeerAddr().GetIpPort() << std::endl;
}

void OnNewRawMsg(ConnectionPtr const & conn, RawMsgPtr const & msg) {
    conn->SendRawData(msg->data(), msg->size());
}



int main() {
    PollLoopPtr loop(new xiaotu::net::PollLoop);
    TcpServer tcp(loop, 65530, 3);

    tcp.SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));
    tcp.SetNewRawMsgCallBk(std::bind(OnNewRawMsg, _1, _2));

    while (1) {
        loop->LoopOnce(100000);
    }
}

