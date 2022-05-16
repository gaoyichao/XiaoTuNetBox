#include <XiaoTuNetBox/Acceptor.h>

#include <unistd.h>

namespace xiaotu {
namespace net {

    AcceptorPtr CreateAcceptor(int port, int qsize, EventLoop const & loop) {
        AcceptorPtr re = AcceptorPtr(new Acceptor(port, qsize, loop));
        return re;
    }


    /*
     * 构造函数
     * 
     * 当有新的连接请求到来，而此时队列已满，客户端将接收到一个ECONNREFUSED的错误
     * 
     * @port: 监听端口
     * @qsize: 等待连接的队列长度
     */
    Acceptor::Acceptor(int port, int qsize, EventLoop const & loop)
        : mAccpSock(AF_INET, SOCK_STREAM, 0)
    {
        IPv4 addr(port);
        mAccpSock.SetReuseAddr(true);
        mAccpSock.SetKeepAlive(true);
        mAccpSock.BindOrDie(addr);
        mAccpSock.ListenOrDie(qsize);

        mEventHandler = loop.CreateEventHandler(mAccpSock.GetFd());
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&Acceptor::OnReadEvent, this));
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
 
