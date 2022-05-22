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
        mRcvdMsg = std::make_shared<WebSocketMsg>();
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

    //! @brief eOpen 状态，获取 ws 消息头
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * WebSocketSession::OnOpen(uint8_t const * begin, uint8_t const * end)
    {
        return nullptr;
        //size_t size = mInBuf->Size();
        //if (size < 2)
        //    return false;

        //uint8_t const * begin = mInBuf->Begin();
        //WsHead const * wshead = (WsHead const *)begin;

        //if (wshead->resv != 0x0 || !IsOpcodeValide(*wshead)) {
        //    mState = eError;
        //    mInBuf->DropAll();
        //    return true;
        //}

        //mRcvdMsg->SetWsHead(*wshead);
        //mInBuf->DropFront(sizeof(WsHead));

        //uint64_t pl_len = wshead->PayloadLen & 127;
        //if (pl_len < 126) {
        //    mRcvdMsg->SetPayloadLen(pl_len);
        //    mState = eReadingMask;
        //    return true;
        //} else if (pl_len == 126) {
        //    mState = eReadingLen16;
        //    return true;
        //} else if (pl_len == 127) {
        //    mState = eReadingLen64;
        //    return true;
        //}

        //return false;
    }

    //! @brief eReadingLen16 状态，获取 ws 帧的扩展长度
    //! @param conn TCP通信连接
    //! @return true 完整解析并切换状态, false 未切换状态
    bool WebSocketSession::OnReadingLen16(ConnectionPtr const & conn)
    {
        return false;
        //size_t size = mInBuf->Size();
        //if (size < 2)
        //    return false;

        //uint8_t const * begin = mInBuf->Begin();
        //uint64_t pl_len = be16toh(*(uint16_t*)begin);
        //mRcvdMsg->SetPayloadLen(pl_len);

        //mInBuf->DropFront(2);
        //mState = eReadingMask;

        //return true;
    }

    bool WebSocketSession::OnReadingLen64(ConnectionPtr const & conn)
    {
        return false;
        //size_t size = mInBuf->Size();
        //if (size < 8)
        //    return false;

        //uint8_t const * begin = mInBuf->Begin();
        //uint64_t pl_len = be64toh(*(uint64_t*)begin) & ~(1ULL << 63);
        //mRcvdMsg->SetPayloadLen(pl_len);

        //mInBuf->DropFront(8);
        //mState = eReadingMask;
        //return true;

    }

    bool WebSocketSession::OnReadingMask(ConnectionPtr const & conn)
    {
        return false;
        //size_t size = mInBuf->Size();
        //if (size < 4)
        //    return false;

        //uint8_t const * begin = mInBuf->Begin();
        //mRcvdMsg->SetMask(*(uint32_t*)begin);

        //mRcvdMsg->PrintMask();

        //mInBuf->DropFront(4);
        //mState = eReadingPayload;
        //DLOG(INFO) << "payloadlen:" << mRcvdMsg->GetPayloadLen();
        //return true;
    }

    bool WebSocketSession::OnReadingPayload(ConnectionPtr const & conn)
    {
        return false;
        //size_t n0 = mRcvdMsg->mPayload.size();
        //size_t need = mRcvdMsg->GetPayloadLen() - n0;

        //size_t n = (need > mInBuf->Size()) ? mInBuf->Size() : need;
        //mRcvdMsg->mPayload.resize(mRcvdMsg->mPayload.size() + n);
        //mInBuf->PopFront(mRcvdMsg->mPayload.data() + n0, n);

        //if (n == need) {
        //    mRcvdMsg->UnMaskPayload();
        //    mState = eNewMsg;
        //    return true;
        //}

        //return false;
    }

    WebSocketMsgPtr WebSocketSession::HandleMsg(uint8_t const * begin, uint8_t const * end)
    {
        while (begin < end) {
        //    if (eOpen == mState) {
        //        if (!OnOpen(conn))
        //            break;
        //    }
        
            break;

        //    if (eReadingLen16 == mState) {
        //        if (!OnReadingLen16(conn))
        //            break;
        //    }

        //    if (eReadingLen64 == mState) {
        //        if (!OnReadingLen64(conn))
        //            break;
        //    }

        //    if (eReadingMask == mState) {
        //        if (!OnReadingMask(conn))
        //            break;
        //    }

        //    if (eReadingPayload == mState)
        //        OnReadingPayload(conn);

        //    if (eNewMsg == mState || eError == mState)
        //        break;
        }

        //if (eNewMsg == mState || eError == mState) {
        //    DLOG(INFO) << GetStateStr();
        //    WebSocketMsgPtr re = mRcvdMsg;
        //    mRcvdMsg = std::make_shared<WebSocketMsg>();
        //    return re;
        //}
        return nullptr;
    }
}
}
