#include <stdio.h>

#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/SerialPort.h>
#include <XiaoTuNetBox/PollLoop.h>

void OnStdinRawMsg(int fd, xiaotu::net::RawMsgPtr const & msg) {
    int n = write(fd, msg->data(), msg->size() - 1);
    if (n < 0)
        perror("发送错误:");
    std::cout << "n = " << n << std::endl;
}

void OnRead(uint8_t const * buf, int num)
{
    for (int i = 0; i < num; i++)
        printf("%c", buf[i]);

    std::cout << "num = " << num << std::endl;
}

int main(int argc, char *argv[])
{
    if (2 != argc) {
        std::cout << "./build/t_SerialPort.exe /dev/ttyUSB0" << std::endl;
        exit(-1);
    }
    std::string dev_path(argv[1]);

    xiaotu::net::PollLoopPtr gLoop = xiaotu::net::CreatePollLoop();

    xiaotu::net::SerialPortPtr port(new xiaotu::net::SerialPort(dev_path.c_str()));
    port->SetReadCB(std::bind(&OnRead, std::placeholders::_1, std::placeholders::_2));
    xiaotu::net::ApplyOnLoop(port, gLoop);

    xiaotu::net::ConnectionPtr stdin_conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(0, "标准输入:stdin:0"));
    stdin_conn->SetRecvRawCallBk(std::bind(&OnStdinRawMsg, port->GetFd(), std::placeholders::_1));
    xiaotu::net::ApplyOnLoop(stdin_conn, gLoop);

    gLoop->Loop(1000);
    return 0;
}



