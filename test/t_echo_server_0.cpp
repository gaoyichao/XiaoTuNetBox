#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Socket.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include <iostream>

int main() {
    xiaotu::net::IPv4 serverIp(65530);
    xiaotu::net::Socket sock(AF_INET, SOCK_STREAM, 0);
    sock.SetReuseAddr(true);
    sock.SetKeepAlive(true);

    int max_conn = 3;
    sock.BindOrDie(serverIp);
    sock.ListenOrDie(2);

    struct pollfd pollFds[max_conn];
    pollFds[0].fd = sock.GetFd();
    pollFds[0].events = POLLRDNORM;
    for (int i = 1; i < max_conn; i++)
        pollFds[i].fd = -1;

    xiaotu::net::IPv4 connections[9];
    char buf[max_conn-1][1024];

    std::cout << "~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    while (1) {
        int nready = poll(pollFds, max_conn, 100000);
        std::cout << "nready = " << nready << std::endl;

        if (POLLRDNORM & pollFds[0].revents) {
            int idx = 1;
            for (; idx < max_conn; idx++) {
                if (pollFds[idx].fd < 0)
                    break;
            }

            std::cout << "idx = " << idx << std::endl;
            int conn_fd = sock.Accept(&connections[idx-1]);
            if (max_conn == idx) {
                std::cout << "连接太多了" << std::endl;
                close(conn_fd);
            } else {
                pollFds[idx].fd = conn_fd;
                pollFds[idx].events = POLLRDNORM;
            }
        }

        for (int i = 1; i < max_conn; i++) {
            if (pollFds[i].fd < 0)
                continue;

            int conn_fd = pollFds[i].fd;
            if (pollFds[i].revents & (POLLRDNORM | POLLERR)) {
                int nread = read(conn_fd, buf[i-1], 1024);
                if (nread <= 0) {
                    std::cout << "close conn_fd = " << conn_fd << std::endl;
                    close(conn_fd);
                    pollFds[i].fd = -1;
                } else {
                    send(conn_fd, buf[i-1], nread, 0);
                }
            }
        }
    }
}

