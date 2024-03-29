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
#include <XiaoTuNetBox/Event.h>
#include <XiaoTuNetBox/InBufObserver.h>

using namespace std::placeholders;

int n_bytes_rcvd = 0;
void OnStdinMsg(int fd, uint8_t const * buf, ssize_t n) {
    n_bytes_rcvd = 0;
    ssize_t re = send(fd, buf, n, 0);

    std::cout << "发送 " << n << " 个字节. re = " << re << std::endl;
}

void OnTcpMsg(uint8_t const * buf, ssize_t n) {
    for (unsigned int i = 0; i < n; i++)
        std::cout << buf[i];

    std::cout << std::endl;

    n_bytes_rcvd += n;
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
    xiaotu::net::ConnectionPtr conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(client_fd, peer_ip, *gLoop));
    conn->SetMsgCallBk(std::bind(&OnTcpMsg, _1, _2));
    xiaotu::net::ApplyOnLoop(conn, gLoop);

    xiaotu::net::ConnectionPtr stdin_conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(0, "标准输入:stdin:0", *gLoop));
    stdin_conn->SetMsgCallBk(std::bind(&OnStdinMsg, client_fd, _1, _2));
    std::cout << stdin_conn->GetInfo() << std::endl;
    xiaotu::net::ApplyOnLoop(stdin_conn, gLoop);

    gLoop->Loop(1000);
    return 0;
}
