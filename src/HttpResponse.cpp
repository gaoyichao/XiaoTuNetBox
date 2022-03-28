#include <XiaoTuNetBox/HttpResponse.h>
#include <XiaoTuNetBox/Utils.h>

#include <iostream>
#include <stdio.h>

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


    void HttpResponse::StartLine(std::string & sl)
    {
        char buf[32];
        snprintf(buf, 32, "HTTP/1.1 %d ", mStatusCode);
        sl.append(buf);
        sl.append(mStatusMessage);
        sl.append("\r\n");
    }

    void HttpResponse::ToUint8Vector(std::vector<uint8_t> & buf)
    {
        char local_buf[64];
        std::string head;
        StartLine(head);
        if (CloseConnection()) {
            head.append("Connection: close\r\n");
        } else {
            snprintf(local_buf, 64, "Content-Length: %zd\r\n", mContent.size());
            head.append(local_buf);
            head.append("Connection: Keep-Alive\r\n");
        }
        for (auto it = mHeaders.begin(); it != mHeaders.end(); ++it)
            head.append(it->first + ": " + it->second + "\r\n");
        head.append("\r\n");

        //std::cout << head << std::endl;

        buf.insert(buf.end(), head.begin(), head.end());
        buf.insert(buf.end(), mContent.begin(), mContent.end());
    }

    void HttpResponse::AppendContent(std::string const & fname, uint64_t off, uint64_t len)
    {
        size_t n_ori = mContent.size();
        
        mContent.resize(n_ori + len);
        ReadBinary(fname, mContent.data() + n_ori, 0, len);
    }
}
}
