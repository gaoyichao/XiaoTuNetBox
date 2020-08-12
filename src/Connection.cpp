#include <XiaoTuNetBox/Connection.h>

#include <functional>
#include <unistd.h>


namespace xiaotu {
namespace net {

    Connection::Connection(int fd, IPv4Ptr const & peer)
        : mPeerAddr(peer)
    {
        mEventHandler = PollEventHandlerPtr(new PollEventHandler(fd));
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&Connection::OnReadEvent, this));
    }


    void Connection::OnReadEvent() {
        int md = mEventHandler->GetFd();
        int nread = read(md, mReadBuf, 1024);
        if (nread <= 0) {
            std::cout << "close fd = " << md << std::endl;
            close(md);
            if (mCloseCallBk)
                mCloseCallBk();
        } else {
            if (mRecvRawCallBk) {
                RawMsgPtr msg(new RawMsg(nread));
                msg->assign(mReadBuf, mReadBuf + nread);
                mRecvRawCallBk(msg);
            }
        }
    }


    void Connection::SendRawData(char const * buf, int num) {
        int md = mEventHandler->GetFd();
        send(md, buf, num, 0);
    }


}
}

