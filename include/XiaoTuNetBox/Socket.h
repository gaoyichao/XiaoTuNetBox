#ifndef XTNB_SOCKET_H
#define XTNB_SOCKET_H

#include <XiaoTuNetBox/Address.h>

namespace xiaotu {
namespace net {
    class Socket {
        public:
            Socket(int fd) : mFd(fd) {}
            Socket(int domain, int type, int protocol);
            ~Socket();
            Socket(Socket const &) = delete;
            Socket & operator = (Socket const &) = delete;

        public:
            bool SetReuseAddr(bool on);
            bool SetKeepAlive(bool on);

            void BindOrDie(IPv4 const & ip);
            void ListenOrDie(int conn);

            int Accept(IPv4 * pear);
            int GetFd() const { return mFd; }
        private:
            int mFd;
    };
}
}

#endif

