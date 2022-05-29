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

    //! @brief WebSocket 消息
    //!
    //! 通过 Parse 解析消息
    class WebSocketMsg {
        public:
            enum EState {
                eReadingHead = 0,
                eReadingLen16 = 1,
                eReadingLen64 = 2,
                eReadingMask = 3,
                eReadingPayload = 4,
                eNewMsg = 5,
                eBuiltMsg = 6,
                eError = 7,
            };

            static std::map<EState, std::string> mEStateToStringMap;

        public:
            WebSocketMsg();
            WebSocketMsg(uint8_t opcode, const uint8_t * payload, uint64_t n);

            EState GetState() const { return mState; }
            inline std::string const & GetStateStr() const
            {
                return mEStateToStringMap[mState];
            }

            //! @brief 在发送之前整理消息头
            //!
            //! @param opcode 操作字, enum EWsOpcode
            void PreSend(uint8_t opcode);

        public:
            uint8_t const * Parse(uint8_t const * begin, uint8_t const * end);
 
        private:
            uint8_t const * OnReadingHead    (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingLen16   (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingLen64   (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingMask    (uint8_t const * begin, uint8_t const * end);
            uint8_t const * OnReadingPayload (uint8_t const * begin, uint8_t const * end);

            uint8_t const * GetData(int n, uint8_t const * & begin, uint8_t const * & end);

            void UnMaskPayload();
            void SetPayloadLen(uint64_t n);
        private:
            EState mState;
            //! 记录已经解析的 payload 数据长度
            size_t mLoadedLen;
            //! 用于缓存 head, length 等数据的结构
            std::vector<uint8_t> mBufData;

        public:
            //! @brief 获取消息头的起始地址
            inline WsHead * GetHead() { return (WsHead *)mContent.data(); }

            //! @brief 获取消息内容长度
            inline uint64_t GetPayloadLen() { return mPayloadLen; }

            //! @brief 获取消息内容的起始地址
            inline uint8_t * GetPayload() { return mContent.data() + mHeadLen; }

            //! @brief 获取完整的消息
            inline std::vector<uint8_t> const & GetContent() const { return mContent; }

        private:
            uint32_t mHeadLen;
            uint64_t mPayloadLen;
            uint8_t mMask[4];
            std::vector<uint8_t> mContent;
    };
    typedef std::shared_ptr<WebSocketMsg> WebSocketMsgPtr;
}
}


#endif
