#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/Acceptor.h>

#include <XiaoTuNetBox/EPollLoop.h>
#include <XiaoTuNetBox/EPollEventHandler.h>

#include <cassert>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

using namespace xiaotu::net;

const int max_conn = 3;                 // 最大连接数量
const int max_evs = 3;                  // 一次最多处理事件数量
struct epoll_event ev;                  // acceptor 的事件对象
struct epoll_event events[max_evs];     // 新建连接的事件对象
char buf[1024];
IPv4 connections[max_conn];

int main() {
    EPollLoopPtr eploop = Create<EPollLoop>();
    int epoll_fd = eploop->GetEpollFd();
    printf("epoll_fd: %d\n", epoll_fd);

    AcceptorPtr acceptor = CreateAcceptor(65530, 3, *eploop);
    EPollEventHandlerPtr ehptr = std::static_pointer_cast<EPollEventHandler>(acceptor->GetHandler());
    ehptr->UseEdgeTrigger(true);
    xiaotu::net::ApplyOnLoop(acceptor, eploop);

    eploop->PreLoop();
    while (1) {
        eploop->LoopOnce(1000);
    }

    return 0;
}
