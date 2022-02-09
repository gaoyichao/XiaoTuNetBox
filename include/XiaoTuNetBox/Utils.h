#ifndef XTNB_UTILS_H
#define XTNB_UTILS_H

#include <stdlib.h>
#include <stdint.h>
#include <string>

namespace xiaotu {
namespace net {

    int GetSockSendBufSize(int fd);
    void SetSockSendBufSize(int fd, int send_buf_size);

    uint32_t RollLeft(uint32_t value, uint32_t bits);
    
    // Be cautious that *in* will be modified and up to 64 bytes will be appended, so make sure in buffer is long enough
    uint32_t Sha1Base64(uint8_t * in, uint64_t in_len, char* out);

    //! @brief 把str中所有大写字母转为小写字母
    void ToLower(std::string & str);

    inline bool IsUpper(char c)
    {
        return 'A' <= c && c <= 'Z';
    }

    inline bool IsLower(char c)
    {
        return 'a' <= c && c <= 'z';
    }

    //! @brief 在 [begin, end) 找到第一个满足 pattern 的地址
    //!
    //! @param begin 待考察的数组起始地址
    //! @param end 待考察的数组终止地址
    //! @param pattern 模板起始地址
    //! @param np 模板长度
    //! @return 第一个匹配的起始地址, 如果没有找到返回NULL
    uint8_t const * FindString(uint8_t const * begin, uint8_t const * end,
                               uint8_t const * pattern, size_t np);

    //! @brief 在 [begin, end) 找到第一个不是 c 的元素
    //!
    //! @param begin 待考察的数组起始地址
    //! @param end 待考察的数组终止地址
    //! @param c 目标字节
    //! @return 第一个不是 c 的元素，若全是 c 则返回 end
    uint8_t const * EatByte(uint8_t const * begin, uint8_t const * end, uint8_t c);

    //! @brief 从 end 开始往回找，找到第一个不是 c 的元素
    //!
    //! @param begin 待考察的数组起始地址
    //! @param end 待考察的数组终止地址
    //! @param c 目标字节
    //! @return 倒数第一个不是 c 的元素，若全是 c 则返回 begin 
    uint8_t const * InvEatByte(uint8_t const * begin, uint8_t const * end, uint8_t c);
}
}

#endif

