#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Socket.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

int main() {
    xiaotu::net::IPv4 serverIp(65530);
    xiaotu::net::Socket sock(AF_INET, SOCK_STREAM, 0);

    sock.BindOrDie(serverIp);
    sock.ListenOrDie(10);

    while (1) {
        xiaotu::net::IPv4 client;
        int conn_fd = sock.Accept(&client);

        if (-1 == conn_fd) {
            perror("连接失败");
            exit(1);
        } else {
            char buf[1024];
            while (1) {
                int nread = read(conn_fd, buf, 1024);
                if (nread <= 0)
                    break;
                std::cout << "nread = " << nread << std::endl;
                send(conn_fd, buf, nread, 0);
            }

            close(conn_fd);
        }
    }
}

