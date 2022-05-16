#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/Acceptor.h>

#include <XiaoTuNetBox/EPollLoop.h>
#include <XiaoTuNetBox/EPollEventHandler.h>
#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/InBufObserver.h>

#include <cassert>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

using namespace std::placeholders;
using namespace xiaotu::net;

const int max_conn = 3;                 // 最大连接数量
const int max_evs = 3;                  // 一次最多处理事件数量
char buf[1024];

EPollLoopPtr eploop = nullptr;
ConnectionPtr conn[max_conn];
InBufObserverPtr obs[max_conn];
int num_conn = 0;

void OnMessage(int idx)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "fd:" << conn[idx]->GetHandler()->GetFd() << std::endl;
    std::cout << "buffer size:" << conn[idx]->GetInputBuffer().Size() << std::endl;

    size_t size = obs[idx]->Size();
    std::vector<uint8_t> msg(size);
    obs[idx]->PopFront(msg.data(), size);
    conn[idx]->SendBytes(msg.data(), msg.size());
}

void OnNewConnection(int fd, IPv4Ptr const &peer_addr) {
    if (num_conn >= max_conn) {
        std::cout << "连接太多了" << std::endl;
        close(fd);
    } else {
        std::cout << "逗你玩" << std::endl;
        conn[num_conn] = std::make_shared<Connection>(fd, peer_addr, *eploop);
        conn[num_conn]->SetMsgCallBk(std::bind(&OnMessage, num_conn));
        obs[num_conn] = conn[num_conn]->GetInputBuffer().CreateObserver();
        ApplyOnLoop(conn[num_conn], eploop);
    }
}

int main() {
    eploop = Create<EPollLoop>();
    int epoll_fd = eploop->GetEpollFd();
    printf("epoll_fd: %d\n", epoll_fd);

    AcceptorPtr acceptor = CreateAcceptor(65530, max_conn, *eploop);
    acceptor->SetNewConnCallBk(std::bind(&OnNewConnection, _1, _2));
    EPollEventHandlerPtr ehptr = std::static_pointer_cast<EPollEventHandler>(acceptor->GetHandler());
    ehptr->UseEdgeTrigger(true);
    xiaotu::net::ApplyOnLoop(acceptor, eploop);

    eploop->PreLoop();
    while (1) {
        eploop->LoopOnce(1000);
    }

    return 0;
}
