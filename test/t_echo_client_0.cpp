#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <cassert>

int main(int argc, char *argv[]) {
    assert(argc == 2);
    std::cout << argv[1] << std::endl;

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == client_fd) {
        perror("创建socket失败");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(65530);
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("连接失败");
        exit(1);
    }

    std::string douniwan;
    char buf[1024];
    while (std::cin >> douniwan) {
        send(client_fd, douniwan.c_str(), douniwan.size(), 0);
        int nread = read(client_fd, buf, 1024);
        if (nread <= 0)
            break;
        buf[nread] = '\0';
        std::cout << buf << std::endl;
    }

    close(client_fd);
    return 0;
}
