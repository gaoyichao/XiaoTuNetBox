#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <XiaoTuNetBox/Utils.h>

namespace xiaotu {
namespace net {

    int GetSockSendBufSize(int fd)
    {
        int send_buf_size = 0;
        socklen_t optlen = sizeof(send_buf_size);
        int err = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, &optlen);
        if (err < 0) {
            perror("获取发送缓冲区大小失败\n");
            exit(1);
        }
    
        return send_buf_size;
    }
    
    void SetSockSendBufSize(int fd, int send_buf_size)
    {
        socklen_t optlen = sizeof(send_buf_size);
        int err = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, optlen);
        if (err < 0) {
            perror("设置发送缓冲区大小失败\n");
            exit(1);
        }
    }

    uint32_t RollLeft(uint32_t value, uint32_t bits)
    {
        return (value << bits) | (value >> (32 - bits));
    }

    // Be cautious that *in* will be modified and up to 64 bytes will be appended, so make sure in buffer is long enough
    uint32_t Sha1Base64(uint8_t * in, uint64_t in_len, char* out) {
        uint32_t h0[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
        uint64_t total_len = in_len;
        in[total_len++] = 0x80;
        int padding_size = (64 - (total_len + 8) % 64) % 64;
        while (padding_size--) in[total_len++] = 0;
        for (uint64_t i = 0; i < total_len; i += 4) {
          uint32_t& w = *(uint32_t*)(in + i);
          w = be32toh(w);
        }
        *(uint32_t*)(in + total_len) = (uint32_t)(in_len >> 29);
        *(uint32_t*)(in + total_len + 4) = (uint32_t)(in_len << 3);
        for (uint8_t* in_end = in + total_len + 8; in < in_end; in += 64) {
          uint32_t* w = (uint32_t*)in;
          uint32_t h[5];
          memcpy(h, h0, sizeof(h));
          for (uint32_t i = 0, j = 0; i < 80; i++, j += 4) {
            uint32_t &a = h[j % 5], &b = h[(j + 1) % 5], &c = h[(j + 2) % 5], &d = h[(j + 3) % 5], &e = h[(j + 4) % 5];
            if (i >= 16) w[i & 15] = RollLeft(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
            if (i < 40) {
              if (i < 20)
                e += ((b & (c ^ d)) ^ d) + 0x5A827999;
              else
                e += (b ^ c ^ d) + 0x6ED9EBA1;
            }
            else {
              if (i < 60)
                e += (((b | c) & d) | (b & c)) + 0x8F1BBCDC;
              else
                e += (b ^ c ^ d) + 0xCA62C1D6;
            }
            e += w[i & 15] + RollLeft(a, 5);
            b = RollLeft(b, 30);
          }
          for (int i = 0; i < 5; i++) h0[i] += h[i];
        }
        const char* base64tb = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        uint32_t triples[7] = {h0[0] >> 8,
                               (h0[0] << 16) | (h0[1] >> 16),
                               (h0[1] << 8) | (h0[2] >> 24),
                               h0[2],
                               h0[3] >> 8,
                               (h0[3] << 16) | (h0[4] >> 16),
                               h0[4] << 8};
        for (uint32_t i = 0; i < 7; i++) {
          out[i * 4] = base64tb[(triples[i] >> 18) & 63];
          out[i * 4 + 1] = base64tb[(triples[i] >> 12) & 63];
          out[i * 4 + 2] = base64tb[(triples[i] >> 6) & 63];
          out[i * 4 + 3] = base64tb[triples[i] & 63];
        }
        out[27] = '=';
        return 28;
    }
}
}



