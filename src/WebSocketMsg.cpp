/************************************************************************************
 * 
 * WebSocketMsg 
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/WebSocketMsg.h>

#include <cassert>
#include <stdio.h>
#include <string.h>

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

    void WebSocketMsg::UnMaskPayload()
    {
        assert(mPayloadLen == mPayload.size());

        if (0 == mHead.MASK)
            return;

        for (uint64_t i = 0; i < mPayloadLen; ++i) {
            mPayload[i] ^= mMask[i & 3];
        }
    }

    std::string WebSocketMsg::MaskString()
    {
        char buf[12];
        sprintf(buf, "%2x:%2x:%2x:%2x", mMask[0], mMask[1], mMask[2], mMask[3]);
        return std::string(buf, strlen(buf));
    }

    void WebSocketMsg::PrintPayload()
    {
        for (size_t i = 0; i < mPayload.size(); ++i)
            printf("%c", mPayload[i]);
        printf("\n");
    }

    RawMsgPtr BuildWsRawMsg(uint8_t opcode, const uint8_t* payload, uint64_t pl_len) {
        uint8_t h[14];
        uint32_t h_len = 2;

        //! @todo fin，处理分包发送

        WsHead * wsh = (WsHead*)h;
        wsh->opcode = opcode & 15;
        wsh->FIN = 1;
        wsh->resv = 0;
        wsh->MASK = 0;

        if (pl_len < 126) {
            wsh->PayloadLen = pl_len;
        } else if (pl_len < 65536) {
            wsh->PayloadLen = 126;
            *(uint16_t*)(h + 2) = htobe16(pl_len);
            h_len += 2;
        } else {
            wsh->PayloadLen = 127;
            *(uint64_t*)(h + 2) = htobe64(pl_len);
            h_len += 8;
        }

        RawMsgPtr msg(new RawMsg(h_len + pl_len));
        for (uint64_t i = 0; i < h_len; i++)
            (*msg)[i] = h[i];
        uint8_t * data = msg->data() + h_len;
        for (uint64_t i = 0; i < pl_len; i++)
            data[i] = payload[i];
        return msg;
    }

}
}
