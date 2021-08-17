#ifndef XTNB_SERIALPORT_H
#define XTNB_SERIALPORT_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include <functional>
#include <cmath>

#include <XiaoTuNetBox/EventHandler.h>

namespace xiaotu {
namespace net {

    class SerialPort {
        public:
            SerialPort(char const * pathname, bool nonblocking = false); 
            ~SerialPort() {
                if (-1 != mModuleFd)
                    close(mModuleFd);
            }
 
            int GetFd() const { return mModuleFd; }
            void SendMsgOrDie(uint8_t const * buf, int size);
            void OnReadEvent();
            xiaotu::net::PollEventHandlerPtr & GetHandler() { return mEventHandler; }
        public:
            typedef std::function<void(uint8_t const * buf, int num)> ReadCB;
            void SetReadCB(ReadCB cb) { mReadCB = std::move(cb); }
            void SetReadErrorCB(ReadCB cb) { mReadErrorCB = std::move(cb); }

            ReadCB mReadCB;
            ReadCB mReadErrorCB;

        private:
            int mModuleFd;
            struct termios mOptions;
            uint8_t mReadBuf[512];

            xiaotu::net::PollEventHandlerPtr mEventHandler;
    };

    typedef std::shared_ptr<SerialPort> SerialPortPtr;
    typedef std::shared_ptr<const SerialPort> SerialPortConstPtr;
}
}

#endif

