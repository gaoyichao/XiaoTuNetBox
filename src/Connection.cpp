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
        SendRawData(mWriteBuf.data(), mWriteBuf.size());
        mWriteBuf.clear();
        mEventHandler->EnableWrite(false);
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

    void Connection::SendBytes(char const *buf, int num) {
        if (mEventHandler->GetLoopTid() == ThreadTools::GetCurrentTid())
            SendRawData(buf, num);
        else {
            auto it = mWriteBuf.end();
            mWriteBuf.insert(it, buf, buf+num);
            std::cout << "writebuf.size = " << mWriteBuf.size() << std::endl;
            mEventHandler->EnableWrite(true);
            mEventHandler->WakeUpLoop();
        }
    }

    void Connection::SendRawMsg(RawMsgPtr const & msg) {
        if (mEventHandler->GetLoopTid() == ThreadTools::GetCurrentTid())
            SendRawData(msg->data(), msg->size());
        else {
            auto it = mWriteBuf.end();
            mWriteBuf.insert(it, msg->begin(), msg->end());
            std::cout << "writebuf.size = " << mWriteBuf.size() << std::endl;
            mEventHandler->EnableWrite(true);
            mEventHandler->WakeUpLoop();
        }
    }

    void Connection::SendRawData(char const * buf, int num) {
        int md = mEventHandler->GetFd();
        send(md, buf, num, 0);
        mEventHandler->EnableWrite(false);
    }


}
}

