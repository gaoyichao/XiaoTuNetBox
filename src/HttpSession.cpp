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
    {
        mRequest = std::make_shared<HttpRequest>();
        mState = eExpectRequestLine;
        mInBuf = conn->GetInputBuffer().CreateObserver();
        //mInBuf->SetRecvCallBk(std::bind(&HttpSession::HandleMsg, this));
    }

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
        
        begin = EatByte(space, end, ' ');
        space = FindString(begin, end, (uint8_t const*)"HTTP/1.", 7);
        if (NULL == space)
            return false;
        mRequest->SetVersion(std::string(begin, begin+8));

        return true;
    }

    void HttpSession::OnExpectRequestLine(ConnectionPtr const & conn)
    {
        size_t n = mInBuf->Size();
        if (n < 8)
            return;

        uint8_t const * begin = mInBuf->Begin();
        uint8_t const * crlf = mInBuf->PeekFor((uint8_t const *)"\r\n", 2);

        if (NULL == crlf) {
            uint8_t const * space = mInBuf->PeekFor((uint8_t const *)" ", 1);
            if (NULL == space || !mRequest->SetMethod(std::string(begin, space))) {
                mState = eError;
                mInBuf->DropAll();
            }
            return;
        }

        if (ParseRequestLine(begin, crlf))
            mState = eReadingHeaders;
        else
            mState = eError;
        mInBuf->DropFront(crlf - begin + 2);
    }

    void HttpSession::OnReadingHeaders(ConnectionPtr const & conn)
    {
        uint8_t const * begin = mInBuf->Begin();
        uint8_t const * crlf = mInBuf->PeekFor((uint8_t const *)"\r\n", 2);

        if (NULL == crlf)
            return;

        if (crlf == begin) {
            mState = eReadingBody;
            mInBuf->DropFront(2);
            return;
        }

        uint8_t const * colon = FindString(begin, crlf, (uint8_t const *)":", 1);
        if (NULL == colon) {
            mState = eError;
            return;
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
    }

    void HttpSession::OnReadingBody(ConnectionPtr const & conn)
    {
        size_t n0 = mRequest->mContent.size();
        size_t need = mRequest->ContentLength() - n0;
        if (0 == need) {
            mState = eResponsing;
            return;
        }

        size_t n = (need > mInBuf->Size()) ? mInBuf->Size() : need;
        mRequest->mContent.resize(mRequest->mContent.size() + n);
        mInBuf->PopFront(mRequest->mContent.data() + n0, n);
    }

    HttpRequestPtr HttpSession::HandleMsg(ConnectionPtr const & conn)
    {
        while (!mInBuf->Empty()) {
            if (eExpectRequestLine == mState)
                OnExpectRequestLine(conn);
            else if (eReadingHeaders == mState)
                OnReadingHeaders(conn);

            if (eReadingBody == mState)
                OnReadingBody(conn);
        }

        if (eResponsing == mState || eError == mState) {
            HttpRequestPtr request = mRequest;
            mRequest = std::make_shared<HttpRequest>();
            return request;
        }
        return nullptr;
    }
    

}
}
