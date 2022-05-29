/************************************************************************************
 * 
 * WebSocketMsg 
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/WebSocketMsg.h>

#include <cassert>
#include <string>
#include <map>

#include <stdio.h>
#include <string.h>

#include <endian.h>
#include <glog/logging.h>


namespace xiaotu {
namespace net {
 
    std::map<uint8_t, std::string> gEWsOpcodeToStringMap = {
        { eWS_OPCODE_CONT,    "continuation frame" },
        { eWS_OPCODE_TEXT,    "text frame" },
        { eWS_OPCODE_BINARY,  "binary frame" },
        { eWS_OPCODE_CLOSE,   "cmd close connnection" },
        { eWS_OPCODE_PING,    "cmd ping" },
        { eWS_OPCODE_PONG,    "cmd pong" },
        { eWS_OPCODE_UNKNOWN, "unknown" },
    };

    std::string const & GetWsOpcodeStr(uint8_t code)
    {
        auto it = gEWsOpcodeToStringMap.find(code);
        if (gEWsOpcodeToStringMap.end() == it)
            return gEWsOpcodeToStringMap[eWS_OPCODE_UNKNOWN];
        return it->second;
    }

    bool IsOpcodeValide(WsHead const & head)
    {
        auto it = gEWsOpcodeToStringMap.find(head.opcode);
        if (gEWsOpcodeToStringMap.end() == it)
            return false;
        return eWS_OPCODE_UNKNOWN != head.opcode;
    }

    std::ostream & operator << (std::ostream & stream, WsHead const & data)
    {
        stream << "FIN:" << ((1 == data.FIN) ? "1" : "0") << std::endl;
    
        if (eWS_OPCODE_CONT == data.opcode)
            stream << "opcode: 0x0: continuation frame" << std::endl;
        if (eWS_OPCODE_TEXT == data.opcode)
            stream << "opcode: 0x1: text frame" << std::endl;
        if (eWS_OPCODE_BINARY == data.opcode)
            stream << "opcode: 0x2: binary frame" << std::endl;
        if (eWS_OPCODE_CLOSE == data.opcode)
            stream << "opcode: 0x8: connection close" << std::endl;
        if (eWS_OPCODE_PING == data.opcode)
            stream << "opcode: 0x9: ping" << std::endl;
        if (eWS_OPCODE_PONG == data.opcode)
            stream << "opcode: 0xA: pong" << std::endl;
    
        stream << "MASK:" << ((1 == data.MASK) ? "1" : "0") << std::endl;
        stream << "PayloadLen:" << (uint32_t)data.PayloadLen << std::endl;
        return stream;
    }

    /********************************************************************************/


    std::map<WebSocketMsg::EState, std::string> WebSocketMsg::mEStateToStringMap = {
        { WebSocketMsg::eReadingHead,    "reading ws head" },
        { WebSocketMsg::eReadingLen16,   "reading ext payloadlen 16bit" },
        { WebSocketMsg::eReadingLen64,   "reading ext payloadlen 64bit" },
        { WebSocketMsg::eReadingMask,    "reading mask" },
        { WebSocketMsg::eReadingPayload, "reading payload" },
        { WebSocketMsg::eNewMsg,         "received new websocket msg" },
        { WebSocketMsg::eBuiltMsg,       "built websocket msg" },
        { WebSocketMsg::eError,          "Error" },
    };

    //! @brief 默认构造函数，一般用于接收消息
    WebSocketMsg::WebSocketMsg()
        : mState(eReadingHead), mContent(2)
    {
        LOG(INFO) << mContent.size();
    }

    //! @brief 构造函数，一般用于构建将要发送的消息
    //!
    //! @param opcode 操作字, enum EWsOpcode
    //! @param payload 消息内容的起始地址，可以为 nullptr，此时消息内容是随机的
    //! @param n 消息内容长度
    WebSocketMsg::WebSocketMsg(uint8_t opcode, const uint8_t * payload, uint64_t n)
        : mState(eBuiltMsg), mContent(2)
    {
        WsHead * wsh = GetHead();
        wsh->opcode = opcode & 15;
        wsh->FIN = 1;
        wsh->resv = 0;
        wsh->MASK = 0;

        SetPayloadLen(n);

        if (nullptr != payload) {
            uint8_t * dst = mContent.data() + mHeadLen;
            memcpy(dst, payload, n);
        }
    }

    //! @brief 设定消息内容长度，将给 mContent 分配合适大小的内存，
    //! 同时，还会维护消息头部。
    //!
    //! @param n 消息内容字节数
    void WebSocketMsg::SetPayloadLen(uint64_t n)
    {
        mHeadLen = 2;
        mPayloadLen = n;

        WsHead * wsh = (WsHead *)mContent.data();
        if (n < 126) {
            wsh->PayloadLen = n;
        } else if (n < 65536) {
            wsh->PayloadLen = 126;
            mHeadLen += 2;
        } else {
            wsh->PayloadLen = 127;
            mHeadLen += 8;
        }
        size_t len = mHeadLen + n;
        mContent.resize(len);

        wsh = (WsHead *)mContent.data();
        uint8_t * plen = mContent.data() + 2;
        if (126 == wsh->PayloadLen) {
            *(uint16_t*)plen = htobe16(n);
        } else if (127 == wsh->PayloadLen) {
            *(uint64_t*)plen = htobe64(n);
        }
    }

    //! @brief 从缓存中获取指定字节长度的数据
    //! 
    //! @param n 字节长度
    //! @param begin [inout] 输入缓存的起始地址，输出数据的起始地址
    //! @param end [inout] 输入缓存的结束地址，输出数据的结束地址 
    //! @return 消费 n 个字节之后的缓存起始地址，
    //!         若缓存中字节不足 n 个，则返回 nullptr，参数 begin 和 end 保持不变
    uint8_t const * WebSocketMsg::GetData(int n, uint8_t const * & begin, uint8_t const * & end)
    {
        int nbuf = end - begin;
        int need = n - mBufData.size();

        if (need <= 0) {
            mBufData.clear();
            mState = eError;
            //! @todo 这个与我们的 return 状态有些不一致
            //! 但是，似乎不影响系统运行
            return nullptr;
        }

        if (nbuf < need) {
            mBufData.insert(mBufData.end(), begin, begin + nbuf); 
            return nullptr;
        }

        uint8_t const * re = begin + need;
        if (!mBufData.empty()) {
            mBufData.insert(mBufData.end(), begin, begin + need);
            begin = (uint8_t const *)mBufData.data();
            end = begin + mBufData.size();
        } else {
            end = re;
        }
        return re;
    }

    //! @brief eReadingHead 状态，获取 ws 消息头
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketMsg::OnReadingHead(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(2, begin, end);
        if (nullptr == crlf)
            return nullptr;

        WsHead const * wshead = (WsHead const *)begin;

        if (wshead->resv != 0x0 || !IsOpcodeValide(*wshead)) {
            mState = eError;
            mBufData.clear();
            return nullptr;
        }

        WsHead * head = (WsHead*)mContent.data();
        *head = *wshead;

        uint64_t pl_len = wshead->PayloadLen & 127;
        if (pl_len < 126) {
            SetPayloadLen(pl_len);
            mState = eReadingMask;
        } else if (pl_len == 126) {
            mState = eReadingLen16;
        } else if (pl_len == 127) {
            mState = eReadingLen64;
        } else {
            // 逻辑上，不可能有这个分支了
            mState = eError;
        }
        mBufData.clear();
        return crlf;
    }


    //! @brief eReadingLen16 状态，获取 ws 帧的扩展长度
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketMsg::OnReadingLen16(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(2, begin, end);
        if (nullptr == crlf)
            return nullptr;

        uint64_t pl_len = be16toh(*(uint16_t*)begin);
        SetPayloadLen(pl_len);

        mState = eReadingMask;
        mBufData.clear();
        return crlf;
    }

    //! @brief eReadingLen64 状态，获取 ws 帧的扩展长度
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketMsg::OnReadingLen64(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(8, begin, end);
        if (nullptr == crlf)
            return nullptr;

        uint64_t pl_len = be64toh(*(uint64_t*)begin) & ~(1ULL << 63);
        SetPayloadLen(pl_len);

        mState = eReadingMask;
        mBufData.clear();
        return crlf;
    }

    //! @brief eReadingMask 状态，获取 ws 帧的验证掩码
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr
    uint8_t const * WebSocketMsg::OnReadingMask(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(4, begin, end);
        if (nullptr == crlf)
            return nullptr;

        *(uint32_t*)mMask = *(uint32_t*)begin;

        mLoadedLen = 0;
        mState = eReadingPayload;
        mBufData.clear();
        return crlf;
    }

    //! @brief eReadingPayload 状态，获取 ws 帧的数据内容
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr
    uint8_t const * WebSocketMsg::OnReadingPayload(uint8_t const * begin, uint8_t const * end)
    {
        size_t nbuf = end - begin;
        size_t n0 = mLoadedLen;
        size_t need = GetPayloadLen() - n0;

        size_t n = (need > nbuf) ? nbuf : need;
        uint8_t const * crlf = begin + n;

        uint8_t * dst = mContent.data() + mHeadLen + mLoadedLen;
        memcpy(dst, begin, n);
        mLoadedLen += n;

        if (n == need) {
            UnMaskPayload();
            mState = eNewMsg;
            return crlf;
        }

        return nullptr;
    }

    //! @brief 解析 ws 消息报文
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketMsg::Parse(uint8_t const * begin, uint8_t const * end)
    {
        while (begin < end) {
            if (eReadingHead == mState) {
                begin = OnReadingHead(begin, end);
                if (nullptr == begin)
                    break;
            }
        
            if (eReadingLen16 == mState) {
                begin = OnReadingLen16(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eReadingLen64 == mState) {
                begin = OnReadingLen64(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eReadingMask == mState) {
                begin = OnReadingMask(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eReadingPayload == mState) {
                begin = OnReadingPayload(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eNewMsg == mState || eError == mState) {
                DLOG(INFO) << GetStateStr();
                DLOG(INFO) << "payloadlen:" << GetPayloadLen();
                break;
            }
        }
        return begin;
    }

    void WebSocketMsg::UnMaskPayload()
    {
        assert((mPayloadLen + mHeadLen) == mContent.size());

        WsHead * head = (WsHead*)mContent.data();
        if (0 == head->MASK)
            return;

        uint8_t * begin = mContent.data() + mHeadLen;
        for (uint64_t i = 0; i < mPayloadLen; ++i) {
            begin[i] ^= mMask[i & 3];
        }
    }

    void WebSocketMsg::PreSend(uint8_t opcode)
    {
        WsHead * wsh = GetHead();
        wsh->opcode = opcode & 15;
        wsh->FIN = 1;
        wsh->resv = 0;
        wsh->MASK = 0;
    }

}
}
