#ifndef XTNB_ADDRESS_H
#define XTNB_ADDRESS_H

#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

namespace xiaotu {
namespace net {
    class IPv4 {
        public:
            /*
             * 默认构造，一般用于服务器监听端口
             */
            IPv4(uint16_t port = 0);
            IPv4(std::string const & ip, uint16_t port);

            sa_family_t GetFamily() const { return mSockAddr.sin_family; }
            std::string GetIp() const;
            uint16_t GetPort() const;
            std::string GetIpPort() const;

            struct sockaddr const * GetSockAddr() const { return (struct sockaddr*)(&mSockAddr); }

            void SetSockAddr(struct sockaddr_in const & addr);
        private:
            struct sockaddr_in mSockAddr;
    };
}
}

#endif

