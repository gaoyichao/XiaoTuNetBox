#include <XiaoTuNetBox/Socket.h>

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

namespace xiaotu {
namespace net {
    /*
     * 构造函数, 封装系统调用socket(2)
     * 
     * @domain: 链路层，通信方式，AF_INET:ipv4, AF_INET6:ipv6, AF_CAN:CAN ......
     * @type: 数据层，套接字类型, SOCK_STREAM: tcp ........
     * @protocol: 应用层，协议，好像都写得0
     */
    Socket::Socket(int domain, int type, int protocol) {
        mFd = socket(domain, type, protocol);
        if (mFd < 0) {
            perror("创建Socket失败");
            exit(1);
        }
    }

    Socket::~Socket() {
        close(mFd);
    }

    bool Socket::SetReuseAddr(bool on) {
        int opt = on ? 1 : 0;
        return 0 == setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    bool Socket::SetKeepAlive(bool on) {
        int opt = on ? 1 : 0;
        return 0 == setsockopt(mFd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
    }


    void Socket::ConnectOrDie(IPv4 const & ip) {
        if (connect(mFd, ip.GetSockAddr(), sizeof(struct sockaddr_in)) < 0) {
            perror("连接失败");
            exit(1);
        }       
    }

    void Socket::BindOrDie(IPv4 const & ip) {
        if (bind(mFd, ip.GetSockAddr(), sizeof(struct sockaddr_in)) < 0) {
            perror("bind failed");
            exit(1);
        }
    }

    void Socket::ListenOrDie(int conn) {
        if (listen(mFd, conn) < 0) {
            perror("listen failed");
            exit(1);
        }
    }

    int Socket::Accept(IPv4 & pear) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int fd = accept(mFd, (struct sockaddr *)&addr, &len);
        pear.SetSockAddr(addr);

        if (fd < 0)
            perror("accept failed");

        return fd;
    }

    int Socket::Accept(IPv4Ptr pear) {
        return Accept(*pear);
    }
}
}

