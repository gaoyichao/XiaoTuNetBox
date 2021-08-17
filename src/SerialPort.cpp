#include <XiaoTuNetBox/SerialPort.h>

namespace xiaotu {
namespace net {

    SerialPort::SerialPort(char const * pathname, bool nonblocking)
        : mModuleFd(-1)
    {
        if (nonblocking)
            mModuleFd = open(pathname, O_RDWR | O_NOCTTY | O_NONBLOCK);
        else
            mModuleFd = open(pathname, O_RDWR | O_NOCTTY);
        if (-1 == mModuleFd) {
            perror("error:");
            assert(false);
        }

        mOptions.c_cflag = 0;
        mOptions.c_iflag = 0;
        mOptions.c_lflag = 0;
        mOptions.c_oflag = 0;

        if (cfsetspeed(&mOptions, B115200) < 0) {
            std::cerr << "setting speed failed" << std::endl;
            exit(-1);
        }

        mOptions.c_cflag |=  CLOCAL;
        mOptions.c_cflag |=  CREAD;
        mOptions.c_cflag &= ~CRTSCTS; // Disable hardware flow control (old)
        mOptions.c_cflag &= ~CSTOPB;
        mOptions.c_cflag &= ~CSIZE;
        mOptions.c_cflag |=  CS8;
        mOptions.c_cflag &= ~PARENB; // No Parity

        mOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // raw, no echoing or waiting till a flush or signals
        mOptions.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control

        mOptions.c_oflag &= ~OPOST; /*Output*/

        tcflush(mModuleFd, TCIOFLUSH);////清空输入输出缓存
        usleep(10000);

        if (tcsetattr(mModuleFd, TCSAFLUSH, &mOptions) < 0) {
            std::cerr << "setting options failed" << std::endl;
            perror("error:");
            assert(false);
        }

        mEventHandler = xiaotu::net::PollEventHandlerPtr(new xiaotu::net::PollEventHandler(mModuleFd));
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&SerialPort::OnReadEvent, this));
    }

    void SerialPort::OnReadEvent() {
        int nread = read(mModuleFd, mReadBuf, 512);
        if (nread > 0) {
            if (mReadCB)
                mReadCB(mReadBuf, nread);
        } else {
            if (mReadErrorCB)
                mReadErrorCB(mReadBuf, nread);
        }
    }

    void SerialPort::SendMsgOrDie(uint8_t const * buf, int size) {
        if(write(mModuleFd, buf, size)==-1) {
            perror("send error");
            exit(-1);
        }
    }


}
}

