#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <chrono>

#include <XiaoTuNetBox/Utils.h>

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

            xiaotu::net::SetSockSendBufSize(conn_fd, 4096);
            int send_buf_size = xiaotu::net::GetSockSendBufSize(conn_fd);
            int sbuf_size = 5253120;
            printf("发送缓冲区大小:%d字节\n", send_buf_size);
            printf("申请发送缓存:%d字节\n", sbuf_size);

            char rcv_buf[1024];
            char * as = (char *)malloc(sbuf_size);
            memset(as, 'A', sbuf_size);
            
            while (1) {
                int nread = read(conn_fd, rcv_buf, 1024);
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
                int nsend = send(conn_fd, as, sbuf_size, 0);
                std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
                std::chrono::duration<double> td = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
                std::cout << "请求发送 " << sbuf_size << " 个字节, 实际发送 " << nsend << " 个字节, 共耗时 " << td.count() * 1000 << " ms" << std::endl; 
                if (nsend < 0)
                    perror("发送出错");

                nsend = send(conn_fd, rcv_buf, nread, 0);
                printf("nsend = %d\n", nsend);
                if (nsend < 0)
                    perror("发送出错");
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
