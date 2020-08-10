#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listen_fd) {
        perror("创建socket失败");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(65530);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen failed");
        exit(1);
    }

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);
        int conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &len);
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
    close(listen_fd);
}

