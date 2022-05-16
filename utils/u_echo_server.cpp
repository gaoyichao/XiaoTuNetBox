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
#include <typeinfo>

using namespace std::placeholders;
using namespace xiaotu::net;

class EchoSession : public Session {
    public:
        EchoSession(ConnectionPtr const & conn)
            : mConn(conn)
        {
            mObserver = conn->GetInputBuffer().CreateObserver();
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

        virtual char const * ToCString() { return typeid(*this).name(); }

    private:
        ConnectionPtr mConn;
        InBufObserverPtr mObserver;
};

typedef std::shared_ptr<EchoSession> EchoSessionPtr;
std::vector<EchoSessionPtr> sessions;

SessionPtr OnNewConnection(ConnectionPtr const & conn) {
    std::cout << "新建连接:" << conn->GetInfo() << std::endl;
    conn->GetHandler()->SetNonBlock(true);

    EchoSessionPtr ptr(new EchoSession(conn));
    sessions.push_back(ptr);
    return ptr;
}

void OnCloseConnection(ConnectionPtr const & conn) {
    std::cout << "关闭连接:" << conn->GetInfo() << std::endl;
}

void OnMessage(ConnectionPtr const & con)
{
    std::cout << "接收到了消息" << std::endl;
    EchoSessionPtr ptr = std::static_pointer_cast<EchoSession>(con->mUserObject.lock());
    std::cout << ptr->ToCString() << std::endl;

    ptr->Echo();
}


int main() {
    PollLoopPtr loop = Create<PollLoop>();
    TcpServer tcp(loop, 65530, 3);

    tcp.SetTimeOut(10, 0, 5);
    tcp.SetNewConnCallBk(std::bind(OnNewConnection, _1));
    tcp.SetCloseConnCallBk(std::bind(OnCloseConnection, _1));
    tcp.SetMessageCallBk(std::bind(OnMessage, _1));

    loop->Loop(10);

    return 0;
}

