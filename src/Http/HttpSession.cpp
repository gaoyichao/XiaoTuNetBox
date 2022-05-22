/************************************************************************************
 * 
 * HttpSession - 一次Http会话
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/Http/HttpSession.h>
#include <XiaoTuNetBox/Utils.h>

#include <cassert>
#include <iostream>
#include <string>
#include <map>

#include <glog/logging.h>

namespace xiaotu {
namespace net {

    std::map<HttpSession::EState, std::string> HttpSession::mEStateToStringMap = {
        { eExpectRequestLine, "Expect Request Line" },
        { eReadingHeaders,    "Reading Headers" },
        { eReadingBody,       "Reading Body" },
        { eResponsing,        "Responsing" },
        { eError,             "Error" },
    };

    HttpSession::HttpSession()
    {
        Reset();
    }

    HttpSession::~HttpSession()
    {
        LOG(INFO) << "释放会话";
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

    //! @brief 从缓存中获取一行字符串
    //! @param begin [inout] 输入缓存的起始地址，输出字符串的起始地址
    //! @param end [inout] 输入缓存的结束地址，输出字符串的结束地址 
    //! @return 消费一行字符串之后的缓存起始地址，
    //!         若消费完所有缓存，仍然没有获得完整的一行，则返回 nullptr 
    //!         若 mReadingLine 过长，认为接收数据帧出错，切换状态 eError
    uint8_t const * HttpSession::GetLine(uint8_t const * & begin, uint8_t const * & end)
    {
        uint8_t const * crlf = FindString(begin, end, (uint8_t const *)"\r\n", 2);

        if (nullptr == crlf) {
            mReadingLine.insert(mReadingLine.end(), begin, end);

            if (mReadingLine.size() > 1024) {
                mReadingLine.clear();
                mState = eError;
                //! @todo 这个与我们的 return 状态有些不一致
                //! 但是，似乎不影响系统运行
            }

            return nullptr;
        }

        uint8_t const * re = crlf + 2;
        if (!mReadingLine.empty()) {
            mReadingLine.insert(mReadingLine.end(), begin, crlf);

            begin = (uint8_t const *)mReadingLine.data();
            end = begin + mReadingLine.size();
        } else {
            end = crlf;
        }
        return re;
    }
    //! @brief 获取请求首行
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费首行之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * HttpSession::OnExpectRequestLine(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * re = GetLine(begin, end);
        if (nullptr == re)
            return nullptr;

        if (ParseRequestLine(begin, end))
            mState = eReadingHeaders;
        else
            mState = eError;

        mReadingLine.clear();
        return re;
    }

    //! @brief 解析请求消息头
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费首部之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * HttpSession::OnReadingHeaders(uint8_t const * begin, uint8_t const * end)
    {
        uint8_t const * re = GetLine(begin, end);
        if (nullptr == re)
            return nullptr;

        if (begin == end) {
            mState = eReadingBody;
            mReadingLine.clear();
            return re;
        }

        uint8_t const * colon = FindString(begin, end, (uint8_t const *)":", 1);
        if (NULL == colon) {
            mState = eError;
            mReadingLine.clear();
            return re;
        }
 
        uint8_t const * key_begin = EatByte(begin, colon, ' ');
        uint8_t const * key_end = colon;
        while (' ' == key_end[-1]) key_end--;
 
        uint8_t const * val_begin = colon + 1;
        while (*val_begin == ' ') val_begin++;
        uint8_t const * val_end = end;
        while (' ' == val_end[-1]) val_end--;

        uint32_t key_len = key_end - key_begin;
        uint32_t val_len = val_end - val_begin;
     
        std::string key((char const *)key_begin, key_len);
        std::string value((char const *)val_begin, val_len);
        mRequest->SetHeader(key, value);

        mReadingLine.clear();
        return re;
    }

    //! @brief 解析请求消息主体
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * HttpSession::OnReadingBody(uint8_t const * begin, uint8_t const * end)
    {
        size_t n0 = mRequest->mContent.size();
        size_t need = mRequest->ContentLength() - n0;

        size_t n = end - begin;
        n = (need > n) ? n : need;
        mRequest->mContent.insert(mRequest->mContent.end(), begin, begin + n);

        if (n == need) {
            mState = eResponsing;
            return begin + n;
        }

        return nullptr;
    }

    //! @brief 解析请求消息报文
    //! @param begin 接收缓存起始地址
    //! @param end 接收缓存结束地址
    //! @return 若切换状态，则返回消费数据之后的起始地址，
    //!         若消费完所有 n 个字节，仍然没有切换状态，则返回 nullptr 
    uint8_t const * HttpSession::HandleRequest(uint8_t const * begin, uint8_t const * end)
    {
        while (begin < end) {
            if (eExpectRequestLine == mState) {
                begin = OnExpectRequestLine(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eReadingHeaders == mState) {
                begin = OnReadingHeaders(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eReadingBody == mState) {
                begin = OnReadingBody(begin, end);
                if (nullptr == begin)
                    break;
            }

            if (eError == mState) {
                mRequest->SetMethod(HttpRequest::eINVALID);
                break;
            }
            if (eResponsing == mState)
                break;
        }

        return begin;
    }
    

}
}
