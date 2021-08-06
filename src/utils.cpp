#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <XiaoTuNetBox/Utils.h>

namespace xiaotu {
namespace net {

    int GetSockSendBufSize(int fd) {
        int send_buf_size = 0;
        socklen_t optlen = sizeof(send_buf_size);
        int err = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, &optlen);
        if (err < 0) {
            perror("获取发送缓冲区大小失败\n");
            exit(1);
        }
    
        return send_buf_size;
    }
    
    void SetSockSendBufSize(int fd, int send_buf_size) {
        socklen_t optlen = sizeof(send_buf_size);
        int err = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, optlen);
        if (err < 0) {
            perror("设置发送缓冲区大小失败\n");
            exit(1);
        }
    }

}
}



