#include <XiaoTuNetBox/WebSocketSession.h>
#include <XiaoTuNetBox/Utils.h>


#include <cassert>
#include <iostream>
#include <string>
#include <map>

namespace xiaotu {
namespace net {

    WebSocketSession::WebSocketSession(ConnectionPtr const & conn)
        : mConn(conn)
    {
        mState = eIdle;
        mObserver = conn->GetInputBuffer().CreateObserver();
        mObserver->SetRecvCallBk(std::bind(&WebSocketSession::HandleMsg, this));
    }

    enum eWsOpcode {
        //! CONTinuation, 分片帧
        eWS_OPCODE_CONT = 0,
        //! 文本
        eWS_OPCODE_TEXT = 1,
        //! 二进制
        eWS_OPCODE_BINARY = 2,
        //! 连接断开
        eWS_OPCODE_CLOSE = 8,
        eWS_OPCODE_PING = 9,
        eWS_OPCODE_PONG = 10
    };

#pragma pack(push, 1)
    struct WsHead {
        //! 操作代码，决定如何解析后续数据
        uint8_t opcode : 4;
        //! 保留
        uint8_t RSV3 : 1;
        uint8_t RSV2 : 1;
        uint8_t RSV1 : 1;
        //! 消息的最后一帧
        uint8_t FIN : 1;
        uint8_t PayloadLen : 7;
        uint8_t MASK : 1;
    };
#pragma pack(pop)

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
        return stream;
    }

    RawMsgPtr BuildWsRawMsg(uint8_t opcode, const uint8_t* payload, uint64_t pl_len) {
        uint8_t h[14];
        uint32_t h_len = 2;
    
        //! @todo fin，处理分包发送
        
        WsHead * wsh = (WsHead*)h;
        wsh->opcode = opcode & 15;
        wsh->FIN = 1;
        wsh->RSV1 = 0;
        wsh->RSV2 = 0;
        wsh->RSV3 = 0;
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

    void WebSocketSession::HandleMsg()
    {
        size_t size = mObserver->Size();
        std::cout << "observer size:" << size << std::endl;
        std::cout << "buffer size:" << mConn->GetInputBuffer().Size() << std::endl;
    
        RawMsgPtr msg(new RawMsg(size));
        mObserver->PopFront(msg->data(), size);
    
        ConnectionPtr const & conn = mConn;
    
        uint8_t const * data = msg->data();
        uint8_t const * data_end = data + size;
        uint8_t const * cursor = data;
        int num_line = 0;
    
        if (eReady == mState) {
            for (int i = 0; i < msg->size(); ++i)
                printf("%2x", msg->data()[i]);
            std::cout << "----" << std::endl;
            uint8_t * data = msg->data();
            uint8_t * data_end = msg->data() + msg->size();
    
            WsHead const * wshead = (WsHead const *)data;
            std::cout << *wshead << std::endl;
    
            data += 2;
            uint64_t pl_len = wshead->PayloadLen & 127;
            if (126 == pl_len) {
                pl_len = be16toh(*(uint16_t *)data);
                data += 2;
            } else if (127 == pl_len) {
                pl_len = be64toh(*(uint64_t*)data) & ~(1ULL << 63);
                data += 8;
            }
            std::cout << "PayloadLen:" << pl_len << std::endl;
    
            uint8_t mask_key[4];
            if (wshead->MASK) {
                *(uint32_t*)mask_key = *(uint32_t*)data;
                data += 4;
            }
            printf("%2x:%2x:%2x:%2x\n", mask_key[0], mask_key[1], mask_key[2], mask_key[3]);
    
            //! @todo 检查数据长度
            
            if (wshead->MASK) {
                for (uint64_t i = 0; i < pl_len; i++) {
                    data[i] ^= mask_key[i & 3];
                    printf("%c", data[i]);
                }
            }
    
            RawMsgPtr smsg = BuildWsRawMsg(eWS_OPCODE_BINARY, data, pl_len);
            conn->SendRawMsg(smsg);
    
            std::cout << "----" << std::endl;
            return;
        }
    
    
        while (data < data_end) {
            uint8_t const * ln = (uint8_t const *)memchr(data, '\n', data_end - data);
            if (!ln)
                return;
            if (*--ln != '\r')
                break;
            
            if (0 == num_line) {
                if (memcmp(data, "GET ", 4))
                    break;
                data += 4;
                while (' ' == data[0])
                    data++;
                mRequestURI.clear();
                while (' ' != data[0]) {
                    mRequestURI.push_back(data[0]);
                    data++;
                }
            } else {
                uint8_t const * val_end = ln;
                while (' ' == val_end[-1])
                    val_end--;
     
                if (val_end == data) {
                    if (mRequestMap.end() == mRequestMap.find("Host"))
                        break;
                    if (mRequestMap.end() == mRequestMap.find("Sec-WebSocket-Key"))
                        break;
    
                    if (mRequestMap.end() == mRequestMap.find("Connection"))
                        break;
                    if ("Upgrade" != mRequestMap["Connection"])
                        break;
    
                    if (mRequestMap.end() == mRequestMap.find("Upgrade"))
                        break;
                    if ("websocket" != mRequestMap["Upgrade"])
                        break;
    
                    if (mRequestMap.end() == mRequestMap.find("Sec-WebSocket-Version"))
                        break;
                    if ("13" != mRequestMap["Sec-WebSocket-Version"])
                        break;
    
                    std::cout << "接收到来自 " << conn->GetInfo() << " 的websocket请求" << std::endl;
                    std::cout << "url:" << mRequestURI << std::endl;
                    std::cout << "host:" << mRequestMap["Host"] << std::endl;
    
                    //std::string wskey = mRequestMap["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                    //std::cout << wskey << std::endl;
                    //std::cout << wskey.size() << std::endl;
    
                    uint8_t wskey[128];
                    std::cout << "--------------------" << std::endl;
                    std::cout << mRequestMap["Sec-WebSocket-Key"] << std::endl;
                    std::cout << mRequestMap["Sec-WebSocket-Key"].size() << std::endl;
                    std::cout << "--------------------" << std::endl;
    
                    memcpy(wskey, mRequestMap["Sec-WebSocket-Key"].data(), mRequestMap["Sec-WebSocket-Key"].size());
                    memcpy(wskey + 24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
     
                    char accept_str[128];
                    int acc_n = Sha1Base64(wskey, 24 + 36, accept_str);
                    accept_str[acc_n] = 0;
                    std::cout << accept_str << std::endl;
    
                    char resp[1024];
                    uint32_t resp_len =  resp_len = sprintf(resp,
                                   "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: "
                                   "Upgrade\r\nSec-WebSocket-Accept: %s\r\n",
                                   accept_str);
                    resp_len += sprintf(resp + resp_len, "\r\n");
     
                    conn->SendBytes((uint8_t const *)resp, resp_len);
                    std::cout << "你在逗我吗" << std::endl;
                    std::cout << resp << std::endl;
                    mState = eReady;
                    return;
                }
                uint8_t const * colon = (uint8_t const *)memchr(data, ':', ln - data);
                if (!colon)
                    break;
     
                uint8_t const * val = colon + 1;
                while (*val == ' ') val++;
                uint32_t key_len = colon - data;
                uint32_t val_len = val_end - val;
     
                std::string key((char const *)data, key_len);
                std::string value((char const *)val, val_len);
                mRequestMap[key] = value;
            }
            data = ln + 2;
            num_line++;
        }
    
        const char* resp400 = "HTTP/1.1 400 Bad Request\r\nSec-WebSocket-Version: 13\r\n\r\n";
        conn->SendBytes((uint8_t const *)resp400, strlen(resp400));
        conn->Close();
    
        //mConn->SendBytes(msg.data(), msg.size());
    }


}
}
