#include <XiaoTuNetBox/HttpSession.h>
#include <XiaoTuNetBox/Utils.h>


#include <cassert>
#include <iostream>
#include <string>
#include <map>

namespace xiaotu {
namespace net {

    std::map<HttpSession::EState, std::string> HttpSession::mEStateToStringMap = {
        { eExpectRequestLine, "Expect Request Line" },
        { eReadingHeaders,    "Reading Headers" },
        { eReadingBody,       "Reading Body" },
        { eResponsing,        "Responsing" },
        { eError,             "Error" },
    };

    HttpSession::HttpSession(ConnectionPtr const & conn)
        : Session(conn)
    {
        Reset();
    }

    HttpSession::~HttpSession()
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":释放会话" << std::endl;
    }

    //! @brief 解析 Http 请求的起始行
    //! @param begin 行的起始地址
    //! @param end 行的结束地址
    //! @return true 成功获取起始行, false 未成
    bool HttpSession::ParseRequestLine(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * space = FindString(begin, end, (uint8_t const *)" ", 1);
        if (NULL == space)
            return false;

        if (!mRequest->SetMethod(std::string(begin, space)))
            return false;

        begin = EatByte(space, end, ' ');
        space = FindString(begin, end, (uint8_t const *)" ", 1);
        if (NULL == space)
            return false;
        mRequest->SetURL(std::string(begin, space));
        uint8_t const * que = FindString(begin, space, (uint8_t const *)"?", 1);
        if (NULL != que) {
            mRequest->SetURLPath(std::string(begin, que));
            mRequest->SetURLQuery(std::string(que, space));
        } else {
            mRequest->SetURLPath(std::string(begin, space));
        }
        
        begin = EatByte(space, end, ' ');
        space = FindString(begin, end, (uint8_t const*)"HTTP/1.", 7);
        if (NULL == space)
            return false;
        mRequest->SetVersion(std::string(begin, begin+8));

        return true;
    }

    //! @brief 获取请求首行
    //! @param conn TCP通信连接
    //! @return true 完整解析并切换状态, false 未切换状态
    bool HttpSession::OnExpectRequestLine(ConnectionPtr const & conn)
    {
        size_t n = mInBuf->Size();
        if (n < 8)
            return false;

        uint8_t const * begin = mInBuf->Begin();
        uint8_t const * crlf = mInBuf->PeekFor((uint8_t const *)"\r\n", 2);

        if (NULL == crlf) {
            uint8_t const * space = mInBuf->PeekFor((uint8_t const *)" ", 1);
            if (NULL == space || !mRequest->SetMethod(std::string(begin, space))) {
                mState = eError;
                mInBuf->DropAll();
            }
            return true;
        }

        if (ParseRequestLine(begin, crlf))
            mState = eReadingHeaders;
        else
            mState = eError;
        mInBuf->DropFront(crlf - begin + 2);
        return true;
    }

    //! @brief 解析请求消息头
    //! @param conn TCP通信连接
    //! @return true 完整解析并切换状态, false 未切换状态
    bool HttpSession::OnReadingHeaders(ConnectionPtr const & conn)
    {
        uint8_t const * begin = mInBuf->Begin();
        uint8_t const * crlf = mInBuf->PeekFor((uint8_t const *)"\r\n", 2);

        if (NULL == crlf)
            return false;

        if (crlf == begin) {
            mState = eReadingBody;
            mInBuf->DropFront(2);
            return true;
        }

        uint8_t const * colon = FindString(begin, crlf, (uint8_t const *)":", 1);
        if (NULL == colon) {
            mState = eError;
            mInBuf->DropAll();
            return true;
        }
 
        uint8_t const * key_begin = EatByte(begin, colon, ' ');
        uint8_t const * key_end = colon;
        while (' ' == key_end[-1]) key_end--;
 
        uint8_t const * val_begin = colon + 1;
        while (*val_begin == ' ') val_begin++;
        uint8_t const * val_end = crlf;
        while (' ' == val_end[-1]) val_end--;

        uint32_t key_len = key_end - key_begin;
        uint32_t val_len = val_end - val_begin;
     
        std::string key((char const *)key_begin, key_len);
        std::string value((char const *)val_begin, val_len);
        mRequest->SetHeader(key, value);

        mInBuf->DropFront(crlf - begin + 2);
        return true;
    }

    //! @brief 解析请求消息主体
    //! @param conn TCP通信连接
    //! @return true 完整解析并切换状态, false 未切换状态
    bool HttpSession::OnReadingBody(ConnectionPtr const & conn)
    {
        size_t n0 = mRequest->mContent.size();
        size_t need = mRequest->ContentLength() - n0;
        if (0 == need) {
            mState = eResponsing;
            return true;
        }

        size_t n = (need > mInBuf->Size()) ? mInBuf->Size() : need;
        mRequest->mContent.resize(mRequest->mContent.size() + n);
        mInBuf->PopFront(mRequest->mContent.data() + n0, n);
        return false;
    }

    HttpRequestPtr HttpSession::HandleRequest(ConnectionPtr const & conn)
    {
        while (!mInBuf->Empty()) {
            if (eExpectRequestLine == mState) {
                if (!OnExpectRequestLine(conn))
                    break;
            }

            if (eReadingHeaders == mState) {
                if (!OnReadingHeaders(conn))
                    break;
            }

            if (eReadingBody == mState)
                OnReadingBody(conn);
        }

        if (eResponsing == mState || eError == mState)
            return mRequest;
        return nullptr;
    }
    

}
}
