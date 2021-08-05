#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>

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

int main(int argc, char *argv[]) {
    assert(argc == 2);
    int buf_size = atoi(argv[1]);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd) {
        perror("创建socket失败");
        exit(1);
    }

    int send_buf_size = GetSockSendBufSize(fd);
    printf("发送缓冲区大小:%d字节\n", send_buf_size);

    printf("尝试修改发送缓存大小:%d字节\n", buf_size);
    SetSockSendBufSize(fd, buf_size);

    send_buf_size = GetSockSendBufSize(fd);
    printf("修改后发送缓冲区大小:%d字节\n", send_buf_size);


    close(fd);
    return 0;
}
