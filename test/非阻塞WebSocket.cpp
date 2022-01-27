/******************************************************************************
 * 
 * 非阻塞echo_polloop - echo服务器例程
 * 
 * 单线程
 * 
 * 对应运行 t_stdin_ipv4_talk
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/TcpServer.h>

#include <functional>
#include <iostream>
#include <string>
#include <map>

#include <endian.h>
#include <string.h>

using namespace std::placeholders;
using namespace xiaotu::net;

class Session {
    public:
        Session(ConnectionPtr const & conn)
            : mConn(conn)
        {
            mObserver = conn->GetInputBuffer().CreateObserver();
            mObserver->SetRecvCallBk(std::bind(&Session::Response, this));
        }

        void Response();
    private:
        ConnectionPtr mConn;
        InBufObserverPtr mObserver;
};

std::string requestURI;
std::map<std::string, std::string> requestMap;

typedef std::shared_ptr<Session> SessionPtr;
std::vector<SessionPtr> sessions;


void OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetInfo() << std::endl;
    conn->GetHandler()->SetNonBlock(true);

    sessions.push_back(SessionPtr(new Session(conn)));
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
}

  static uint32_t rol(uint32_t value, uint32_t bits) { return (value << bits) | (value >> (32 - bits)); }
  // Be cautious that *in* will be modified and up to 64 bytes will be appended, so make sure in buffer is long enough
  static uint32_t sha1base64(uint8_t * in, uint64_t in_len, char* out) {
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
        if (i >= 16) w[i & 15] = rol(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
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
        e += w[i & 15] + rol(a, 5);
        b = rol(b, 30);
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

bool inited = false;

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

#pragma pack(push, 4)
struct ws_head {
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

std::ostream & operator << (std::ostream & stream, ws_head const & data)
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

RawMsgPtr SendWsData(uint8_t opcode, const uint8_t* payload, uint64_t pl_len) {
    uint8_t h[14];
    uint32_t h_len = 2;

    //! @todo fin，处理分包发送
    
    ws_head * wsh = (ws_head*)h;
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

void Session::Response()
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


    if (inited) {
        for (int i = 0; i < msg->size(); ++i)
            printf("%2x", msg->data()[i]);
        std::cout << "----" << std::endl;
        uint8_t * data = msg->data();
        uint8_t * data_end = msg->data() + msg->size();

        ws_head const * wshead = (ws_head const *)data;
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

        RawMsgPtr smsg = SendWsData(eWS_OPCODE_BINARY, data, pl_len);
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
            requestURI.clear();
            while (' ' != data[0]) {
                requestURI.push_back(data[0]);
                data++;
            }
        } else {
            uint8_t const * val_end = ln;
            while (' ' == val_end[-1])
                val_end--;
 
            if (val_end == data) {
                if (requestMap.end() == requestMap.find("Host"))
                    break;
                if (requestMap.end() == requestMap.find("Sec-WebSocket-Key"))
                    break;

                if (requestMap.end() == requestMap.find("Connection"))
                    break;
                if ("Upgrade" != requestMap["Connection"])
                    break;

                if (requestMap.end() == requestMap.find("Upgrade"))
                    break;
                if ("websocket" != requestMap["Upgrade"])
                    break;

                if (requestMap.end() == requestMap.find("Sec-WebSocket-Version"))
                    break;
                if ("13" != requestMap["Sec-WebSocket-Version"])
                    break;

                std::cout << "接收到来自 " << conn->GetInfo() << " 的websocket请求" << std::endl;
                std::cout << "url:" << requestURI << std::endl;
                std::cout << "host:" << requestMap["Host"] << std::endl;

                //std::string wskey = requestMap["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                //std::cout << wskey << std::endl;
                //std::cout << wskey.size() << std::endl;

                uint8_t wskey[128];
                std::cout << "--------------------" << std::endl;
                std::cout << requestMap["Sec-WebSocket-Key"] << std::endl;
                std::cout << requestMap["Sec-WebSocket-Key"].size() << std::endl;
                std::cout << "--------------------" << std::endl;

                memcpy(wskey, requestMap["Sec-WebSocket-Key"].data(), requestMap["Sec-WebSocket-Key"].size());
                memcpy(wskey + 24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
 
                char accept_str[128];
                int acc_n = sha1base64(wskey, 24 + 36, accept_str);
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
                inited = true;
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
            requestMap[key] = value;
        }
        data = ln + 2;
        num_line++;
    }

    const char* resp400 = "HTTP/1.1 400 Bad Request\r\nSec-WebSocket-Version: 13\r\n\r\n";
    conn->SendBytes((uint8_t const *)resp400, strlen(resp400));
    conn->Close();

    //mConn->SendBytes(msg.data(), msg.size());
}

int main() {
    PollLoopPtr loop = CreatePollLoop();
    TcpServer tcp(loop, 65530, 3);

    tcp.SetTimeOut(10, 0, 5);
    tcp.SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));

    loop->Loop(1000);

    return 0;
}

