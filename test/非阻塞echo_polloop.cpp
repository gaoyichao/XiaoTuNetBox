/******************************************************************************
 * 
 * t_echo_server_0 - echo服务器例程
 * 
 * 单线程
 * 
 * 对应运行t_echo_client_0
 * 
 *****************************************************************************/

#include <XiaoTuNetBox/TcpServer.h>
#include <XiaoTuNetBox/InBufObserver.h>

#include <memory>
#include <vector>
#include <functional>

using namespace std::placeholders;
using namespace xiaotu::net;

class Session {
    public:
        Session(ConnectionPtr const & conn)
            : mConn(conn)
        {
            mObserver = conn->GetInputBuffer().CreateObserver();
            mObserver->SetRecvCallBk(std::bind(&Session::Echo, this));
        }

        void Echo()
        {
            size_t size = mObserver->Size();
            std::cout << "observer size:" << size << std::endl;
            std::cout << "buffer size:" << mConn->GetInputBuffer().Size() << std::endl;

            std::vector<uint8_t> msg(size);
            mObserver->PopFront(msg.data(), size);

            for (unsigned int i = 0; i < msg.size(); i++)
                std::cout << msg[i];
            std::cout << std::endl;

            mConn->SendBytes(msg.data(), msg.size());
        }

    private:
        ConnectionPtr mConn;
        InBufObserverPtr mObserver;
};

typedef std::shared_ptr<Session> SessionPtr;
std::vector<SessionPtr> sessions;

void OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetInfo() << std::endl;
    conn->GetHandler()->SetNonBlock(true);

    sessions.push_back(SessionPtr(new Session(conn)));
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
}


int main() {
    PollLoopPtr loop = CreatePollLoop();
    TcpServer tcp(loop, 65530, 3);

    tcp.SetTimeOut(10, 0, 5);
    tcp.SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));

    loop->Loop(10);

    return 0;
}

