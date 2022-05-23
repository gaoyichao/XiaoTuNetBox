#include <XiaoTuNetBox/Http/HttpResponse.h>
#include <XiaoTuNetBox/Utils.h>

#include <iostream>
#include <stdio.h>
#include <string.h>

#include <glog/logging.h>

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

    void HttpResponse::Reset(bool close)
    {
        mStatusCode = eUnknown;
        mHeaders.clear();
        mDataSize = 0;
        mContent.clear();
        mCloseConnection = close;
        mHeadEndIdx = -1;

        mLoadCount = 0;
        mFilePtr = NULL;
    }

    void HttpResponse::LockHead(size_t size)
    {
        mDataSize = size;
        mHeadEndIdx = 0;
        mContent.clear();

        AppendContent(StartLine());
        AppendContent("Content-Length: ");
        AppendContent(std::to_string(mDataSize));

        if (CloseConnection()) {
            AppendContent("\r\nConnection: close\r\n");
        } else {
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

    bool HttpResponse::SetFile(std::string const & fname)
    {
        assert(mHeadEndIdx < 0);
        if (-1 == stat(fname.c_str(), &mFileStat) || !S_ISREG(mFileStat.st_mode))
            return false;

        mLoadCount = 0;
        mFilePath = fname;
        return true;
    }

    void HttpResponse::LoadContent(int len)
    {
        assert(mHeadEndIdx > 0);
        if (NULL == mFilePtr)
            mFilePtr = fopen(mFilePath.c_str(), "r");
        LOG(INFO) << mFilePath << mLoadCount;
        assert(NULL != mFilePtr);

        size_t need = mHeadEndIdx + len;
        if (mContent.size() < need)
            mContent.resize(need);

        uint8_t * begin = mContent.data() + mHeadEndIdx;
        size_t aclen = fread((void*)begin, 1, len, mFilePtr);
        mDataSize -= aclen;
        mLoadCount++;

        if (aclen < len) {
            fclose(mFilePtr);
            mFilePtr = NULL;
        }
    }
}
}
