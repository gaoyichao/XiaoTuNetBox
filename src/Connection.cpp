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
        mIsClosed = false;
    }

    void Connection::Close() {
        mEventHandler->SetClosing(true);
    }

    void Connection::OnWriteEvent() {
        assert(!mWriteBuf.Empty());

        int n = mWriteBuf.Folded() ? mWriteBuf.FoldedHead() : mWriteBuf.Size();
        int nsend = SendRawData(mWriteBuf.GetBeginAddr(), n);

        mWriteBuf.DropFront(nsend);

        if (mWriteBuf.Empty())
            mEventHandler->EnableWrite(false);
        else
            mEventHandler->EnableWrite(true);
    }

    void Connection::OnReadEvent() {
        int md = mEventHandler->GetFd();
        size_t n = mReadBuf.Read(md);

        if (n <= 0) {
            Close();
        } else {
            if (mMsgCallBk)
                mMsgCallBk();
        }
    }

    void Connection::OnClosingEvent() {
        int md = mEventHandler->GetFd();
        close(md);
        if (mCloseCallBk)
            mCloseCallBk();
        mIsClosed = true;
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

    void Connection::SendString(std::string const & msg)
    {
        SendBytes((uint8_t const *)msg.c_str(), msg.size());
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

