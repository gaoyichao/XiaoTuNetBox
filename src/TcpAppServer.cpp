/************************************************************************************
 * 
 * TcpAppServer - 应用层服务器
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/TcpAppServer.h>

#include <functional>

namespace xiaotu {
namespace net {

    TcpAppServer::TcpAppServer(PollLoopPtr const & loop, int port, int max_conn)
    {
        mServer = std::make_shared<TcpServer>(loop, port, max_conn);
    }

    TcpAppServer::TcpAppServer(TcpServerPtr const & server)
        : mServer(server)
    { }


    int TcpAppServer::GetFreeSessionIdx()
    {
        int idx = 0;
        if (mHoles.empty()) {
            idx = mSessions.size();
            mSessions.push_back(nullptr);
        } else {
            idx = mHoles.back();
            mHoles.pop_back();
            mSessions[idx] = nullptr;
        }
        return idx;
    }

    SessionPtr TcpAppServer::AddSession(SessionPtr const & ptr)
    {
        int idx = GetFreeSessionIdx();
        ptr->mIdx = idx;
        mSessions[idx] = ptr;
        return ptr;
    }

    void TcpAppServer::ReleaseSession(SessionPtr const & ptr)
    {
        assert(nullptr != ptr);
        assert(mSessions[ptr->mIdx] == ptr);

        ptr->ReleaseWakeUpper(mServer->GetPollLoop());
        ptr->mInBuf->Release();

        size_t & idx = ptr->mIdx;
        mSessions[idx].reset();
        mHoles.push_back(idx);
    }

    //! @brief 替换会话对象
    //!
    //! @param ori 原始会话对象
    //! @param ptr 替身
    //! @return 替身, nullptr 若出错
    SessionPtr TcpAppServer::ReplaceSession(SessionPtr const & ori, SessionPtr const & ptr)
    {
        assert(nullptr != ori && nullptr != ptr);
        assert(mSessions[ori->mIdx] == ori);

        int idx = ori->mIdx;
        ptr->mWakeUpper = std::move(ori->mWakeUpper);
        ptr->mInBuf = std::move(ori->mInBuf);

        ptr->mIdx = idx;
        mSessions[idx] = ptr;
        return ptr;
    }

}
}
