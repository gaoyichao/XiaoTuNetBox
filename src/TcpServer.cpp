#include <XiaoTuNetBox/TcpServer.h>
#include <cassert>

namespace xiaotu {
namespace net {

    using namespace std::placeholders;

    TcpServer::TcpServer(PollLoopPtr const & loop, int port, int max_conn)
        : mLoop(loop),
          mMaxConn(max_conn),
          mConnNum(0)
    {
        mAcceptor = CreateAcceptor(port, max_conn);
        mAcceptor->SetNewConnCallBk(std::bind(&TcpServer::OnNewConnection, this, _1, _2));
        ApplyOnLoop(mAcceptor, mLoop);

        mTimeWheel.push_back(new ConnectionNode);
    }

    void TcpServer::SetTimeOut(time_t sec, long nsec, int n) {
        assert(n > 1);

        for (int i = 1; i < n; i++)
            mTimeWheel.push_back(new ConnectionNode);
        mTimer = TimerPtr(new Timer());

        struct timespec t;
        t.tv_sec = sec;
        t.tv_nsec = nsec;
        mTimer->RunEvery(t, std::bind(&TcpServer::OnTimeOut, this));

        ApplyOnLoop(mTimer, mLoop);
    }

    void TcpServer::OnTimeOut() {
        ConnectionNode * head = mTimeWheel.front();
        for (ConnectionNode * it = head->next; it != head; it = it->next) {
            it->conn->Close();
        }

        mTimeWheel.pop_front();
        mTimeWheel.push_back(head);
    }

    void TcpServer::OnNewConnection(int fd, IPv4Ptr const &peer_addr) {
        if (mConnNum >= mMaxConn) {
            std::cout << "连接太多了" << std::endl;
            close(fd);
        } else {
            ConnectionPtr conn(new Connection(fd, peer_addr));
            ApplyOnLoop(conn, mLoop);

            ConnectionNode * node = new ConnectionNode();
            node->conn = conn;
            ConnectionNode * head = mTimeWheel.back();
            AddTail(node, head);

            if (mNewConnCallBk)
                mNewConnCallBk(conn);

            conn->SetCloseCallBk(std::bind(&TcpServer::OnCloseConnection, this, node));
            conn->SetMsgCallBk(std::bind(&TcpServer::OnMessage, this, node));

            mConnNum++;
        }
    }

    void TcpServer::OnCloseConnection(ConnectionNode * con) {
        std::cout << __FUNCTION__ << ":" << con->conn->GetHandler()->GetFd() << std::endl;
        if (mCloseConnCallBk)
            mCloseConnCallBk(con->conn);

        UnApplyOnLoop(con->conn, mLoop);
        Delete(con);
        delete con;
        mConnNum--;
    }

    void TcpServer::OnMessage(ConnectionNode * con)
    {
        if (mMessageCallBk)
            mMessageCallBk(con->conn);

        ConnectionNode * head = mTimeWheel.back();
        Delete(con);
        AddTail(con, head);
    }

}
}

