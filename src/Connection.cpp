#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/ThreadTools.h>

#include <functional>
#include <unistd.h>
#include <cassert>

#include <sys/uio.h>

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

        int iovcnt = 3;
        struct iovec vec[3];
        uint8_t extrabuf[1024];

        vec[0].iov_base = mReadBuf.Empty() ? mReadBuf.GetStorBeginAddr() : mReadBuf.GetEndAddr();
        vec[0].iov_len = mReadBuf.FreeTail();
        vec[1].iov_base = mReadBuf.GetStorBeginAddr();
        vec[1].iov_len = mReadBuf.FreeHead();
        vec[2].iov_base = extrabuf;
        vec[2].iov_len = 1024;

        size_t n = readv(md, vec, iovcnt);
        if (n <= 0) {
            std::cout << "close fd = " << md << std::endl;
            close(md);
            if (mCloseCallBk)
                mCloseCallBk();
        } else {
            int ava = mReadBuf.Available();
            if (n > ava) {
                mReadBuf.AcceptBack(ava);
                mReadBuf.PushBack(extrabuf, n - ava);
            } else {
                mReadBuf.AcceptBack(n);
            }

            if (mRecvRawCallBk) {
                RawMsgPtr msg(new RawMsg(mReadBuf.Size()));
                mReadBuf.PopFront(msg->data(), mReadBuf.Size());
                mRecvRawCallBk(msg);
            }
        }
    }

    void Connection::OnClosingEvent() {
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

