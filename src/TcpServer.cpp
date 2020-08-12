#include <XiaoTuNetBox/TcpServer.h>


namespace xiaotu {
namespace net {

using namespace std::placeholders;

    TcpServer::TcpServer(PollLoopPtr const & loop, int port, int max_conn)
        : mLoop(loop),
          mMaxConn(max_conn),
          mAcceptor(new Acceptor(port, max_conn))
    {
        mAcceptor->SetNewConnCallBk(std::bind(&TcpServer::OnNewConnection, this, _1, _2));
        ApplyHandlerOnLoop(mAcceptor->GetHandler(), mLoop);
    }


    void TcpServer::OnNewConnection(int fd, IPv4Ptr const &peer_addr) {
        if (mConnList.size() >= mMaxConn) {
            std::cout << "连接太多了" << std::endl;
            close(fd);
        } else {
            ConnectionPtr conn(new Connection(fd, peer_addr));
            conn->SetCloseCallBk(std::bind(&TcpServer::OnCloseConnection, this, conn));
            conn->SetRecvRawCallBk(std::bind(&TcpServer::OnNewRawMsg, this, conn, _1));

            ApplyHandlerOnLoop(conn->GetHandler(), mLoop);
            mConnList.push_back(conn);

            if (mNewConnCallBk)
                mNewConnCallBk(conn);
        }
    }

    void TcpServer::OnCloseConnection(ConnectionPtr const & con) {
        for (auto it = mConnList.begin(); it != mConnList.end(); it++) {
            ConnectionPtr & ptr = *it;
            if (ptr == con) {
                UnApplyHandlerOnLoop(ptr->GetHandler(), mLoop);
                mConnList.erase(it);

                if (mCloseConnCallBk)
                    mCloseConnCallBk(con);
                break;
            }
        }
    }

    void TcpServer::OnNewRawMsg(ConnectionPtr const & con, RawMsgPtr const & msg) {
        if (mNewRawMsgCallBk)
            mNewRawMsgCallBk(con, msg);
    }





}
}

