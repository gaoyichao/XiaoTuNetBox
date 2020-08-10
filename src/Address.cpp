#include <XiaoTuNetBox/Address.h>

#include <string.h>

namespace xiaotu {
namespace net {
    IPv4::IPv4(uint16_t port) {
        memset(&mSockAddr, 0, sizeof(mSockAddr));
 
        mSockAddr.sin_family = AF_INET;
        mSockAddr.sin_port = htons(port);
        mSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    IPv4::IPv4(std::string const & ip, uint16_t port) {
        memset(&mSockAddr, 0, sizeof(mSockAddr));
 
        mSockAddr.sin_family = AF_INET;
        mSockAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &mSockAddr.sin_addr) <= 0) {
            std::cout << "地址错误" << std::endl;
            exit(1);
        }
    }

    std::string IPv4::GetIp() const {
        char buf[64] = "";
        inet_ntop(AF_INET, &mSockAddr.sin_addr, buf, sizeof buf);
        return buf;
    }

    uint16_t IPv4::GetPort() const {
        return ntohs(mSockAddr.sin_port);
    }

    std::string IPv4::GetIpPort() const {
        return GetIp() + ":" + std::to_string(GetPort());
    }

    void IPv4::SetSockAddr(struct sockaddr_in const &addr) {
        mSockAddr = addr;
    }
}
}
