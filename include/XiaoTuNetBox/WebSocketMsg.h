/************************************************************************************
 * 
 * WebSocketMsg
 * 
 ***********************************************************************************/
#ifndef XTNB_WEB_SOCKET_MSG_H
#define XTNB_WEB_SOCKET_MSG_H

#include <XiaoTuNetBox/Types.h>

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>


namespace xiaotu {
namespace net {

    enum EWsOpcode {
        //! CONTinuation, 分片帧
        eWS_OPCODE_CONT = 0,
        //! 文本
        eWS_OPCODE_TEXT = 1,
        //! 二进制
        eWS_OPCODE_BINARY = 2,
        //! 连接断开
        eWS_OPCODE_CLOSE = 8,
        eWS_OPCODE_PING = 9,
        eWS_OPCODE_PONG = 10,

        eWS_OPCODE_UNKNOWN
    };

    std::string const & GetWsOpcodeStr(uint8_t code);

#pragma pack(push, 1)
    struct WsHead {
        //! 操作代码，决定如何解析后续数据
        uint8_t opcode : 4;
        //! 保留
        uint8_t resv : 3;
        //! 消息的最后一帧
        uint8_t FIN : 1;
        uint8_t PayloadLen : 7;
        uint8_t MASK : 1;
    };
#pragma pack(pop)

    std::ostream & operator << (std::ostream & stream, WsHead const & data);
    bool IsOpcodeValide(WsHead const & head);

    RawMsgPtr BuildWsRawMsg(uint8_t opcode, const uint8_t* payload, uint64_t pl_len);

    class WebSocketMsg {
        public:
            void SetWsHead(WsHead const & head) { mHead = head; }
            WsHead const & GetWsHead() const { return mHead; }

            void SetPayloadLen(uint64_t len)
            {
                mPayloadLen = len;
                mPayload.reserve(mPayloadLen);
                mPayload.clear();
            }
            inline uint64_t GetPayloadLen() { return mPayloadLen; }

            inline RawMsgPtr BuildWsRawMsg(uint8_t opcode = EWsOpcode::eWS_OPCODE_BINARY)
            {
                return xiaotu::net::BuildWsRawMsg(opcode, mPayload.data(), mPayloadLen);
            }

            void SetMask(uint32_t mask) { *(uint32_t*)mMask = mask; }
            std::string MaskString();
            void PrintPayload();
            void UnMaskPayload();

            size_t LeftPayloadLen() { return mPayloadLen - mPayload.size(); }

        public:
            WsHead mHead;
            uint64_t mPayloadLen;
            uint8_t mMask[4];
            std::vector<uint8_t> mPayload;
    };
    typedef std::shared_ptr<WebSocketMsg> WebSocketMsgPtr;
}
}


#endif
