#include <XiaoTuNetBox/Address.h>
#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/Acceptor.h>
#include <XiaoTuNetBox/Connection.h>

#include <functional>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include <iostream>
#include <list>


const int max_conn = 3;
struct pollfd pollFds[max_conn + 1];
typedef xiaotu::net::ConnectionPtr ConnectionPtr;
ConnectionPtr connections[max_conn];

namespace xiaotu {
namespace net {
    using namespace std::placeholders;
    class TcpServer {
        public:
            TcpServer(PollLoopPtr const & loop, int port, int max_conn)
                : mLoop(loop),
                  mMaxConn(max_conn),
                  mAcceptor(new Acceptor(port, max_conn))
            {
                mAcceptor->SetNewConnCallBk(std::bind(&TcpServer::OnNewConnection, this, _1, _2));
                ApplyHandlerOnLoop(mAcceptor->GetHandler(), mLoop);
            }

            void OnNewConnection(int fd, IPv4Ptr const &peer_addr) {
                if (mConnList.size() >= mMaxConn) {
                    std::cout << "连接太多了" << std::endl;
                    close(fd);
                } else {
                    ConnectionPtr conn(new Connection(fd, peer_addr));
                    conn->SetCloseCallBk(std::bind(&TcpServer::OnCloseConnection, this, conn));
                    ApplyHandlerOnLoop(conn->GetHandler(), mLoop);
                    mConnList.push_back(conn);
                }
            }

            void OnCloseConnection(ConnectionPtr const & con) {
                for (auto it = mConnList.begin(); it != mConnList.end(); it++) {
                    ConnectionPtr & ptr = *it;
                    if (ptr == con) {
                        UnApplyHandlerOnLoop(ptr->GetHandler(), mLoop);
                        mConnList.erase(it);
                        break;
                    }
                }
            }

            Acceptor & GetAcceptor() { return *mAcceptor; }
        private:
            PollLoopPtr mLoop;
            int mMaxConn;
            std::shared_ptr<Acceptor> mAcceptor;
            std::list<ConnectionPtr> mConnList;

    };
}
}


int main() {
    xiaotu::net::PollLoopPtr loop(new xiaotu::net::PollLoop);
    xiaotu::net::TcpServer tcp(loop, 65530, max_conn);

    while (1) {
        loop->LoopOnce(1000);
    }
}

