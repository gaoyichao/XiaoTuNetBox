/******************************************************************************
 * 
 * t_back_server_0 - 后台数据服务器例程
 * 
 * 在主线程定时通过TcpServer发送消息
 * 在thread1中查询PollLoop
 * 
 * 对应运行t_echo_client_0
 * 
 *****************************************************************************/


#include <XiaoTuNetBox/TcpServer.h>
#include <thread>
#include <mutex>
#include <functional>

using namespace std::placeholders;
using namespace xiaotu::net;


PollLoopPtr loop;
std::shared_ptr<TcpServer> tcp;
ConnectionPtr newconn;
std::mutex gmutex;

void OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetInfo() << std::endl;
    std::lock_guard<std::mutex> guard(gmutex);
    newconn = conn;
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
    std::lock_guard<std::mutex> guard(gmutex);
    if (conn == newconn)
        newconn.reset();
}

void OnNewRawMsg(ConnectionPtr const & conn, RawMsgPtr const & msg) {
    conn->SendRawMsg(msg);
}

void Loop() {
    loop->Loop(10000);
}


int main() {
    loop = CreatePollLoop();
    tcp = std::shared_ptr<TcpServer>(new TcpServer(loop, 65530, 3));

    tcp->SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp->SetCloseConnCallBk(std::bind(OnCloseConnection, _1));
    tcp->SetNewRawMsgCallBk(std::bind(OnNewRawMsg, _1, _2));

    std::thread thread1(Loop);

    while (1) {
        std::cout << "douniwan" << std::endl;

        xiaotu::net::RawMsgPtr msg = xiaotu::net::RawMsgPtr(new xiaotu::net::RawMsg);
        msg->push_back('d');
        msg->push_back('d');
        msg->push_back('\n');

        {
            std::lock_guard<std::mutex> guard(gmutex);
            if (newconn) {
                newconn->SendRawMsg(msg);
            }
        }
        sleep(1);
    }

    thread1.join();
    return 0;
}

