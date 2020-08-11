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


const int max_conn = 3;
struct pollfd pollFds[max_conn + 1];
typedef xiaotu::net::ConnectionPtr ConnectionPtr;
ConnectionPtr connections[max_conn];

namespace xiaotu {
namespace net {
    using namespace std::placeholders;
    class TcpServer {
        public:
            TcpServer(int port, int max_conn)
                : mMaxConn(max_conn),
                  mAcceptor(new Acceptor(port, max_conn))
            {
                mAcceptor->SetNewConnCallBk(std::bind(&TcpServer::OnNewConnection, this, _1, _2));

            }

            void OnNewConnection(int fd, IPv4Ptr const &peer_addr) {
                if (mConnList.size() >= mMaxConn) {
                    std::cout << "连接太多了" << std::endl;
                    close(fd);
                } else {
                    ConnectionPtr conn(new Connection(fd, peer_addr));
                    conn->SetCloseCallBk(std::bind(&TcpServer::OnCloseConnection, this, _1));
                    mConnList.push_back(conn);
                }
            }

            void OnCloseConnection(Connection const * con) {
                std::cout << __FUNCTION__ << std::endl;
                for (auto it = mConnList.begin(); it != mConnList.end(); it++) {
                    ConnectionPtr & ptr = *it;
                    if (&(*ptr) == con) {
                        mConnList.erase(it);
                        break;
                    }
                }
            }

            void Run(int timeout) {
                while (1) {
                    std::vector<struct pollfd> pollFdList;
                    std::vector<PollEventHandler*> handlerList;

                    pollFdList.push_back(mAcceptor->GetHandler().GetPollFd());
                    handlerList.push_back(&(mAcceptor->GetHandler()));

                    for (int i = 0; i < mConnList.size(); i++) {
                        ConnectionPtr & ptr = mConnList[i];
                        pollFdList.push_back(ptr->GetHandler().GetPollFd());
                        handlerList.push_back(&(ptr->GetHandler()));
                    }

                    int nready = poll(pollFdList.data(), pollFdList.size(), timeout);
                    std::cout << "nready = " << nready << std::endl;
                    for (int i = 0; i < pollFdList.size(); i++) {
                        if (pollFdList[i].fd < 0)
                            continue;
                        handlerList[i]->HandleEvents(pollFdList[i]);
                    }
                }
            }

            Acceptor & GetAcceptor() { return *mAcceptor; }
        private:
            int mMaxConn;
            std::shared_ptr<Acceptor> mAcceptor;
            std::vector<ConnectionPtr> mConnList;

    };
}
}


int main() {
    xiaotu::net::TcpServer tcp(65530, max_conn);

    tcp.Run(1000);
}

