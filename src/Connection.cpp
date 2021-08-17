#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/ThreadTools.h>

#include <functional>
#include <unistd.h>
#include <cassert>


namespace xiaotu {
namespace net {

    Connection::Connection(int fd, std::string const & info)
        : mInfoStr(info)
    {
        SetFd(fd);
    }

    Connection::Connection(int fd, IPv4Ptr const & peer)
        : mInfoStr(peer->GetIpPort())
    {
        SetFd(fd);
    }

    void Connection::SetFd(int fd) {
        mEventHandler = PollEventHandlerPtr(new PollEventHandler(fd));
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&Connection::OnReadEvent, this));
        mEventHandler->SetWriteCallBk(std::bind(&Connection::OnWriteEvent, this));
        mEventHandler->SetClosingCallBk(std::bind(&Connection::OnClosingEvent, this));
    }

    void Connection::Close() {
        mEventHandler->SetClosing(true);
    }

    void Connection::OnWriteEvent() {
        assert(!mWriteBuf.Empty());

        int n = mWriteBuf.Folded() ? mWriteBuf.FoldedHead() : mWriteBuf.Size();
        int nsend = SendRawData(mWriteBuf.GetBeginAddr(), n);
        std::cout << __FUNCTION__ << ":" << n  << ":" << nsend<< std::endl;

        mWriteBuf.DropFront(nsend);

        if (mWriteBuf.Empty())
            mEventHandler->EnableWrite(false);
        else
            mEventHandler->EnableWrite(true);
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

    void Connection::OnClosingEvent() {
        std::cout << __FUNCTION__ << std::endl;
        int md = mEventHandler->GetFd();
        close(md);
        if (mCloseCallBk)
            mCloseCallBk();
    }

    void Connection::SendBytes(uint8_t const *buf, int num) {
        int nsend = 0;
        if (mEventHandler->GetLoopTid() == ThreadTools::GetCurrentTid() && mWriteBuf.Empty()) {
            nsend = SendRawData(buf, num);
            num -= nsend;
            buf += nsend;
        }

        if (num > 0) {
            mWriteBuf.PushBack(buf, num);
            std::cout << "writebuf.size = " << mWriteBuf.Size() << std::endl;
            mEventHandler->EnableWrite(true);
            if (mEventHandler->GetLoopTid() != ThreadTools::GetCurrentTid())
                mEventHandler->WakeUpLoop();
        }
    }

    void Connection::SendRawMsg(RawMsgPtr const & msg) {
        SendBytes(msg->data(), msg->size());
    }

    int Connection::SendRawData(uint8_t const * buf, int num) {
        int md = mEventHandler->GetFd();
        int nsend = send(md, buf, num, 0);

        if (nsend < 0) {
            nsend = 0;
            int eno = errno;
            if (!(EAGAIN & eno || EWOULDBLOCK & eno))
                perror("发送出错");
        }
        return nsend;
    }


}
}

