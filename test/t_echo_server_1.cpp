#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <chrono>

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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
        int conn_fd = accept4(listen_fd, (struct sockaddr *)&cli_addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (-1 != conn_fd) {
            int fl = fcntl(conn_fd, F_GETFL);
            printf("fl: 0x%x, O_RDWR: 0x%x, O_NONBLOCK: 0x%x\n", fl, O_RDWR, O_NONBLOCK);

            char buf[1024];
            while (1) {
                int nread = read(conn_fd, buf, 1024);
                if (-1 == nread) {
                    int eno = errno;
                    if (EAGAIN & eno || EWOULDBLOCK & eno)
                        continue;
                    else
                        break;
                }

                if (0 == nread)
                    break;

                printf("nread = %d\n", nread);

                std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
                for (int i = 0; i < 100; i++)
                    send(conn_fd, buf, nread, 0);
                std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
                std::chrono::duration<double> td = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
                std::cout << "send td: " << td.count() << std::endl;
            }

            close(conn_fd);
        } else {
            int eno = errno;
            if (EAGAIN & eno || EWOULDBLOCK & eno)
                continue;

            perror("连接失败");
            exit(1);
        }
    }
    close(listen_fd);
}
