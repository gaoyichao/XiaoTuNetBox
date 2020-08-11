#include <XiaoTuNetBox/Acceptor.h>

#include <functional>
#include <unistd.h>

namespace xiaotu {
namespace net {

    Acceptor::Acceptor(int port, int qsize)
        : mAccpSock(AF_INET, SOCK_STREAM, 0),
          mEventHandler(mAccpSock.GetFd())
    {
        IPv4 addr(port);
        mAccpSock.SetReuseAddr(true);
        mAccpSock.SetKeepAlive(true);
        mAccpSock.BindOrDie(addr);
        mAccpSock.ListenOrDie(qsize);

        mEventHandler.EnableRead(true);
        mEventHandler.EnableWrite(false);
        mEventHandler.SetReadCallBk(std::bind(&Acceptor::OnReadEvent, this));
    }

    void Acceptor::OnReadEvent() {
        IPv4Ptr pear_addr = IPv4Ptr(new IPv4);
        int fd = mAccpSock.Accept(pear_addr);
        if (mNewConnCallBk) {
            mNewConnCallBk(fd, pear_addr);
        } else {
            std::cout << "未注册new conn callback" << std::endl;
            ::close(fd);
        }
    }




}
}
 
