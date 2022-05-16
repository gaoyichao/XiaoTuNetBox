#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Socket.h>

#include <cassert>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

const int max_conn = 3;                 // 最大连接数量
const int max_evs = 3;                  // 一次最多处理事件数量
struct epoll_event ev;                  // acceptor 的事件对象
struct epoll_event events[max_evs];     // 新建连接的事件对象
char buf[1024];
xiaotu::net::IPv4 connections[max_conn];

int main() {
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    printf("epoll_fd: %d\n", epoll_fd);

    xiaotu::net::IPv4 serverIp(65530);
    xiaotu::net::Socket sock(AF_INET, SOCK_STREAM, 0);
    sock.SetReuseAddr(true);
    sock.SetKeepAlive(true);
    sock.BindOrDie(serverIp);
    sock.ListenOrDie(max_conn);

    ev.events = EPOLLIN;
    ev.data.fd = sock.GetFd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock.GetFd(), &ev) == -1) {
        perror("epoll_ctl: EPOLL_CTL_ADD");
        exit(-1);
    }

    while (1) {
        int nready = epoll_wait(epoll_fd, events, max_evs, -1);
        printf("nready = %d\n", nready);

        if (nready == -1) {
            perror("epoll_wait");
            exit(-1);
        }

        for (int i = 0; i < nready; ++i) {
            if (events[i].data.fd == sock.GetFd()) {
                assert(events[i].events & EPOLLIN);

                int idx = 0;
                for (; idx < max_conn; ++idx)
                    if (0 == connections[idx].GetPort())
                        break;

                int conn_fd = sock.Accept(connections[idx]);
                if (max_conn == idx) {
                    printf("连接太多了\n");
                    close(conn_fd);
                } else {
                    int fl = fcntl(conn_fd, F_GETFL);
                    fl |= O_NONBLOCK;
                    if (-1 == fcntl(conn_fd, F_SETFL, fl)) {
                        perror("修改NONBLOCK出错");
                        return false;
                    }

                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                        perror("epoll_ctl: conn_sock");
                        exit(EXIT_FAILURE);
                    }
                }
            } else {
                if (EPOLLIN & events[i].events) {
                    int conn_fd = events[i].data.fd;
                    int nread = read(conn_fd, buf, 1024);
                    if (nread <= 0) {
                        std::cout << "close conn_fd = " << conn_fd << std::endl;
                        close(conn_fd);
                    } else {
                        send(conn_fd, buf, nread, 0);
                    }
                }
            }
        }


    }

    close(epoll_fd);
    return 0;
}
