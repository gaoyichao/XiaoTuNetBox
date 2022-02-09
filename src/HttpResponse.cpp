#include <XiaoTuNetBox/HttpResponse.h>
#include <XiaoTuNetBox/Utils.h>

#include <iostream>
#include <stdio.h>

namespace xiaotu {
namespace net {
    std::map<HttpResponse::EStatusCode, std::string> HttpResponse::mEStatusCodeToStringMap = {
        { eUnknown,               "Unknown" },
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
        head.append("\r\n");

        buf.insert(buf.end(), head.begin(), head.end());
        buf.insert(buf.end(), mContent.begin(), mContent.end());
    }

}
}
