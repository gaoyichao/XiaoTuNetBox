#ifndef XTNB_UTILS_H
#define XTNB_UTILS_H

#include <stdint.h>

namespace xiaotu {
namespace net {

    int GetSockSendBufSize(int fd);
    void SetSockSendBufSize(int fd, int send_buf_size);

    uint32_t RollLeft(uint32_t value, uint32_t bits);
    
    // Be cautious that *in* will be modified and up to 64 bytes will be appended, so make sure in buffer is long enough
    uint32_t Sha1Base64(uint8_t * in, uint64_t in_len, char* out);
}
}

#endif

