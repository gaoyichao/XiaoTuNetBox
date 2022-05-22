#include <XiaoTuNetBox/WebSocketSession.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <iostream>
#include <string>
#include <map>

#include <endian.h>
#include <glog/logging.h>

namespace xiaotu {
namespace net {

    std::map<WebSocketSession::EState, std::string> WebSocketSession::mEStateToStringMap = {
        { eConnecting,     "Connecting" },
        { eOpen,           "Open" },
        { eClosing,        "Closing" },
        { eClosed,         "Closed" },
        { eReadingLen16,   "reading ext payloadlen 16bit" },
        { eReadingLen64,   "reading ext payloadlen 64bit" },
        { eReadingMask,    "reading mask" },
        { eReadingPayload, "reading payload" },
        { eNewMsg,         "received new websocket msg" },
        { eError,          "Error" },
    };


    WebSocketSession::WebSocketSession()
        : Session()
    {
        mState = eConnecting;
    }

    WebSocketSession::~WebSocketSession()
    {
        LOG(INFO) << "释放会话";
    }

    bool WebSocketSession::CheckHandShake(HttpRequestPtr const & req)
    {
        std::string value;

        if (!req->GetHeader("Connection", value))
            return false;
        if ("Upgrade" != value)
            return false;

        if (!req->GetHeader("Upgrade", value))
            return false;
        if ("websocket" != value)
            return false;

        std::string host;
        if (!req->GetHeader("Host", host))
            return false;

        std::string secversion, seckey;
        if (!req->GetHeader("Sec-WebSocket-Version", secversion))
            return false;
        if ("13" != secversion)
            return false;

        if (!req->HasHeader("Sec-WebSocket-Key"))
            return false;

        return true;
    }

    bool WebSocketSession::AcceptHandShake(HttpRequestPtr const & req)
    {
        assert(CheckHandShake(req));

        req->GetHeader("Sec-WebSocket-Key", mSecKey);
        uint8_t wskey[128];
        memcpy(wskey, mSecKey.data(), mSecKey.size());
        memcpy(wskey + 24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

        char accept_str[128];
        int acc_n = Sha1Base64(wskey, 24 + 36, accept_str);
        accept_str[acc_n] = 0;
        mAccKey = std::string(accept_str);

        mHandShakeRequest = req;
        mHandShakeResponse = std::make_shared<HttpResponse>();

        mHandShakeResponse->SetClosing(false);
        mHandShakeResponse->SetStatusCode(HttpResponse::e101_SwitchProtocol);
        mHandShakeResponse->SetHeader("Connection", "Upgrade");
        mHandShakeResponse->SetHeader("Upgrade", "websocket");
        mHandShakeResponse->SetHeader("Sec-WebSocket-Accept", mAccKey);

        return true;
    }

    //! @brief 从缓存中获取指定字节长度的数据
    //! 
    //! @param n 字节长度
    //! @param begin [inout] 输入缓存的起始地址，输出数据的起始地址
    //! @param end [inout] 输入缓存的结束地址，输出数据的结束地址 
    //! @return 消费 n 个字节之后的缓存起始地址，
    //!         若缓存中字节不足 n 个，则返回 nullptr，参数 begin 和 end 保持不变
    uint8_t const * WebSocketSession::GetData(int n, uint8_t const * & begin, uint8_t const * & end)
    {
        int nbuf = end - begin;
        int need = n - mReadingData.size();

        if (need <= 0) {
            mReadingData.clear();
            mState = eError;
            //! @todo 这个与我们的 return 状态有些不一致
            //! 但是，似乎不影响系统运行
            return nullptr;
        }

        if (nbuf < need) {
            mReadingData.insert(mReadingData.end(), begin, begin + nbuf); 
            return nullptr;
        }

        uint8_t const * re = begin + need;
        if (!mReadingData.empty()) {
            mReadingData.insert(mReadingData.end(), begin, begin + need);

            begin = (uint8_t const *)mReadingData.data();
            end = begin + mReadingData.size();
        } else {
            end = re;
        }
        return re;
    }

    //! @brief eOpen 状态，获取 ws 消息头
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketSession::OnOpen(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(2, begin, end);
        if (nullptr == crlf)
            return nullptr;

        WsHead const * wshead = (WsHead const *)begin;

        if (wshead->resv != 0x0 || !IsOpcodeValide(*wshead)) {
            mState = eError;
            mReadingData.clear();
            return nullptr;
        }

        mRcvdMsg = std::make_shared<WebSocketMsg>();
        mRcvdMsg->SetWsHead(*wshead);

        uint64_t pl_len = wshead->PayloadLen & 127;
        if (pl_len < 126) {
            mRcvdMsg->SetPayloadLen(pl_len);
            mState = eReadingMask;
        } else if (pl_len == 126) {
            mState = eReadingLen16;
        } else if (pl_len == 127) {
            mState = eReadingLen64;
        } else {
            // 逻辑上，不可能有这个分支了
            mState = eError;
        }
        mReadingData.clear();
        return crlf;
    }

    //! @brief eReadingLen16 状态，获取 ws 帧的扩展长度
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketSession::OnReadingLen16(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(2, begin, end);
        if (nullptr == crlf)
            return nullptr;

        uint64_t pl_len = be16toh(*(uint16_t*)begin);
        mRcvdMsg->SetPayloadLen(pl_len);

        mState = eReadingMask;
        mReadingData.clear();
        return crlf;
    }

    uint8_t const * WebSocketSession::OnReadingLen64(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(8, begin, end);
        if (nullptr == crlf)
            return nullptr;

        uint64_t pl_len = be64toh(*(uint64_t*)begin) & ~(1ULL << 63);
        mRcvdMsg->SetPayloadLen(pl_len);

        mState = eReadingMask;
        mReadingData.clear();
        return crlf;
    }

    //! @brief eReadingMask 状态，获取 ws 帧的验证掩码
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr
    uint8_t const * WebSocketSession::OnReadingMask(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * crlf = GetData(4, begin, end);
        if (nullptr == crlf)
            return nullptr;

        mRcvdMsg->SetMask(*(uint32_t*)begin);

        mState = eReadingPayload;
        mReadingData.clear();
        return crlf;
    }

    //! @brief eReadingPayload 状态，获取 ws 帧的数据内容
    //!
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr
    uint8_t const * WebSocketSession::OnReadingPayload (uint8_t const * begin, uint8_t const * end)
    {
        size_t nbuf = end - begin;
        size_t n0 = mRcvdMsg->mPayload.size();
        size_t need = mRcvdMsg->GetPayloadLen() - n0;

        size_t n = (need > nbuf) ? nbuf : need;
        uint8_t const * crlf = begin + n;
        mRcvdMsg->mPayload.insert(mRcvdMsg->mPayload.end(), begin, crlf);

        if (n == need) {
            mRcvdMsg->UnMaskPayload();
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
    uint8_t const * WebSocketSession::HandleMsg(uint8_t const * begin, uint8_t const * end)
    {
        while (begin < end) {
            if (eOpen == mState) {
                begin = OnOpen(begin, end);
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
                DLOG(INFO) << "Mask:" << mRcvdMsg->MaskString();
                DLOG(INFO) << "payloadlen:" << mRcvdMsg->GetPayloadLen();
                break;
            }
        }

        LOG(INFO) << GetStateStr() << std::endl;
        return begin;
    }
}
}
