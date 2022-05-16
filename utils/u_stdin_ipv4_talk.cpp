#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/InBufObserver.h>

xiaotu::net::InBufObserverPtr stdin_obs;
xiaotu::net::InBufObserverPtr tcp_obs;

int n_bytes_rcvd = 0;
void OnStdinMsg(int fd, xiaotu::net::InBufObserverPtr const & obs) {
    n_bytes_rcvd = 0;
    std::cout << obs->Size() << std::endl;

    size_t size = obs->Size();
    std::vector<uint8_t> msg(size);
    obs->PopFront(msg.data(), size);

    send(fd, msg.data(), msg.size()-1, 0);
}

void OnTcpMsg(xiaotu::net::InBufObserverPtr const & obs) {
    size_t size = obs->Size();
    std::vector<uint8_t> msg(size);
    obs->PopFront(msg.data(), size);

    for (unsigned int i = 0; i < msg.size(); i++)
        std::cout << msg[i];

    std::cout << std::endl;

    n_bytes_rcvd += msg.size();
    std::cout << "接收到 " << n_bytes_rcvd << " 个字节" << std::endl;
}

int main(int argc, char * argv[]) {
    if (3 != argc) {
        std::cout << "./build/t_stdin_ipv4_talk.exe server_ipv4_addr server_port" << std::endl;
        exit(-1);
    }
    std::string ip(argv[1]);
    int port = atoi(argv[2]);

    xiaotu::net::PollLoopPtr gLoop = xiaotu::net::Create<xiaotu::net::PollLoop>();

    xiaotu::net::Socket socket;
    xiaotu::net::IPv4Ptr peer_ip = xiaotu::net::IPv4Ptr(new xiaotu::net::IPv4(ip, port));
    socket.ConnectOrDie(*peer_ip);
    int client_fd = socket.GetFd();
    xiaotu::net::ConnectionPtr conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(client_fd, peer_ip));
    tcp_obs = conn->GetInputBuffer().CreateObserver();
    conn->SetMsgCallBk(std::bind(&OnTcpMsg, tcp_obs));
    xiaotu::net::ApplyOnLoop(conn, gLoop);

    xiaotu::net::ConnectionPtr stdin_conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(0, "标准输入:stdin:0"));
    stdin_obs = stdin_conn->GetInputBuffer().CreateObserver();
    stdin_conn->SetMsgCallBk(std::bind(&OnStdinMsg, client_fd, stdin_obs));
    std::cout << stdin_conn->GetInfo() << std::endl;
    xiaotu::net::ApplyOnLoop(stdin_conn, gLoop);

    gLoop->Loop(1000);
    return 0;
}
