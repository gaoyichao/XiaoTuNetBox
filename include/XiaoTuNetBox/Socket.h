#ifndef XTNB_SOCKET_H
#define XTNB_SOCKET_H

#include <XiaoTuNetBox/Address.h>

namespace xiaotu {
namespace net {
    class Socket {
        public:
            Socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
            ~Socket();
            Socket(Socket const &) = delete;
            Socket & operator = (Socket const &) = delete;

        public:
            bool SetReuseAddr(bool on);
            bool SetKeepAlive(bool on);

            void ConnectOrDie(IPv4 const & ip);
            void BindOrDie(IPv4 const & ip);
            void ListenOrDie(int conn);

            int Accept(IPv4 & pear);
            int Accept(IPv4Ptr pear);
            int GetFd() const { return mFd; }
        private:
            int mFd;
    };
}
}

#endif

