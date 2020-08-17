#include <XiaoTuNetBox/Socket.h>

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

namespace xiaotu {
namespace net {
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

    int Socket::Accept(IPv4Ptr pear) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int fd = accept(mFd, (struct sockaddr *)&addr, &len);
        pear->SetSockAddr(addr);

        if (fd < 0)
            perror("accept failed");

        return fd;
    }
}
}

