#include <XiaoTuNetBox/Http/HttpResponse.h>
#include <XiaoTuNetBox/Utils.h>

#include <iostream>
#include <stdio.h>
#include <string.h>

namespace xiaotu {
namespace net {
    std::map<HttpResponse::EStatusCode, std::string> HttpResponse::mEStatusCodeToStringMap = {
        { eUnknown,               "Unknown" },
        { e101_SwitchProtocol,    "Switching Protocols"},
        { e200_OK,                "OK" },
        { e400_BadRequest,        "Bad Request" },
        { e404_NotFound,          "Not Found" },
        { e503_ServiceUnavilable, "Service Unavilable" },
    };


    const size_t _buf_len_ = 64;
    char _cstr_buf_[_buf_len_];
    std::string HttpResponse::StartLine()
    {
        std::string sl;
        snprintf(_cstr_buf_, _buf_len_, "HTTP/1.1 %d ", mStatusCode);

        sl.append(_cstr_buf_);
        sl.append(GetStatusCodeStr());
        sl.append("\r\n");
        return sl;
    }

    void HttpResponse::AppendContent(char const * cstr)
    {
        size_t len = strlen(cstr);
        AppendContent((uint8_t const*)cstr, len);
    }

    void HttpResponse::AppendContent(std::string const & fname, uint64_t off, uint64_t len)
    {
        size_t n_ori = mContent.size();
        
        mContent.resize(n_ori + len);
        ReadBinary(fname, mContent.data() + n_ori, 0, len);
    }

    void HttpResponse::Reset(bool close)
    {
        mStatusCode = eUnknown;
        mHeaders.clear();
        mDataSize = 0;
        mContent.clear();
        mCloseConnection = close;
        mHeadEndIdx = -1;
    }

    void HttpResponse::LockHead(size_t size)
    {
        mDataSize = size;
        mHeadEndIdx = 0;
        mContent.clear();

        AppendContent(StartLine());
        if (CloseConnection()) {
            AppendContent("Connection: close\r\n");
        } else {
            AppendContent("Content-Length: ");
            AppendContent(std::to_string(mDataSize));
            AppendContent("\r\nConnection: Keep-Alive\r\n");
        }

        for (auto it = mHeaders.begin(); it != mHeaders.end(); ++it)
            AppendContent(it->first + ": " + it->second + "\r\n");
        AppendContent("\r\n");

        mHeadEndIdx = mContent.size();
    }

    void HttpResponse::UnlockHead()
    {
        mDataSize = 0;
        mHeadEndIdx = -1;
        mContent.clear();
    }
}
}
