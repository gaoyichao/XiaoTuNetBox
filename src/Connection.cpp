#include <XiaoTuNetBox/Connection.h>

#include <functional>
#include <unistd.h>


namespace xiaotu {
namespace net {

    Connection::Connection(int fd, IPv4Ptr const & peer)
        : mFd(fd), mPeerAddr(peer), mEventHandler(fd)
    {
        mEventHandler.EnableRead(true);
        mEventHandler.EnableWrite(false);
        mEventHandler.SetReadCallBk(std::bind(&Connection::OnReadEvent, this));
    }


    void Connection::OnReadEvent() {
        int nread = read(mFd, mReadBuf, 1024);
        if (nread <= 0) {
            std::cout << "close fd = " << mFd << std::endl;
            close(mFd);
            if (mCloseCallBk)
                mCloseCallBk(this);
        } else {
            send(mFd, mReadBuf, nread, 0);
        }
    }


}
}

