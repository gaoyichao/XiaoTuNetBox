#ifndef XTNB_UTILS_H
#define XTNB_UTILS_H

namespace xiaotu {
namespace net {

    int GetSockSendBufSize(int fd);
    void SetSockSendBufSize(int fd, int send_buf_size);

}
}

#endif

