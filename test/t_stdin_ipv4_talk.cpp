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

int n_bytes_rcvd = 0;
void OnStdinRawMsg(int fd, xiaotu::net::RawMsgPtr const & msg) {
    n_bytes_rcvd = 0;
    send(fd, msg->data(), msg->size()-1, 0);
}

void OnTcpRawMsg(xiaotu::net::RawMsgPtr const & msg) {
    for (unsigned int i = 0; i < msg->size(); i++)
        std::cout << (*msg)[i];

    std::cout << std::endl;

    n_bytes_rcvd += msg->size();
    std::cout << "接收到 " << n_bytes_rcvd << " 个字节" << std::endl;
}

int main(int argc, char * argv[]) {
    if (3 != argc) {
        std::cout << "./build/t_stdin_ipv4_talk.exe server_ipv4_addr server_port" << std::endl;
        exit(-1);
    }
    std::string ip(argv[1]);
    int port = atoi(argv[2]);

    xiaotu::net::PollLoopPtr gLoop = xiaotu::net::CreatePollLoop();

    xiaotu::net::Socket socket;
    xiaotu::net::IPv4Ptr peer_ip = xiaotu::net::IPv4Ptr(new xiaotu::net::IPv4(ip, port));
    socket.ConnectOrDie(*peer_ip);
    int client_fd = socket.GetFd();
    xiaotu::net::ConnectionPtr conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(client_fd, peer_ip));
    conn->SetRecvRawCallBk(std::bind(&OnTcpRawMsg, std::placeholders::_1));

    xiaotu::net::ApplyOnLoop(conn, gLoop);

    xiaotu::net::ConnectionPtr stdin_conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(0, "标准输入:stdin:0"));
    stdin_conn->SetRecvRawCallBk(std::bind(&OnStdinRawMsg, client_fd, std::placeholders::_1));
    std::cout << stdin_conn->GetInfo() << std::endl;

    xiaotu::net::ApplyOnLoop(stdin_conn, gLoop);

    gLoop->Loop(1000);
    return 0;
}
