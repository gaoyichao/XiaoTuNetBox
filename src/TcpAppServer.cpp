/************************************************************************************
 * 
 * TcpAppServer - 应用层服务器
 * 
 ***********************************************************************************/
#include <XiaoTuNetBox/TcpAppServer.h>

#include <functional>

namespace xiaotu {
namespace net {

    TcpAppServer::TcpAppServer(EventLoopPtr const & loop, int port,
                               int max_conn, std::string const & ws)
    {
        mServer = std::make_shared<TcpServer>(loop, port, max_conn);
        mWorkSpace = ws;
    }


    int TcpAppServer::GetFreeHandlerIdx()
    {
        int idx = 0;
        if (mHoles.empty()) {
            idx = mHandlers.size();
            mHandlers.push_back(nullptr);
        } else {
            idx = mHoles.back();
            mHoles.pop_back();
            mHandlers[idx] = nullptr;
        }
        return idx;
    }

    HandlerPtr TcpAppServer::AddHandler(HandlerPtr const & ptr)
    {
        int idx = GetFreeHandlerIdx();
        ptr->mIdx = idx;
        mHandlers[idx] = ptr;
        return ptr;
    }

    void TcpAppServer::ReleaseHandler(HandlerPtr const & ptr)
    {
        assert(nullptr != ptr);
        assert(mHandlers[ptr->mIdx] == ptr);

        ptr->ReleaseWakeUpper(mServer->GetLoop());
        //ptr->mInBuf->Release();

        size_t & idx = ptr->mIdx;
        mHandlers[idx].reset();
        mHoles.push_back(idx);
    }

    //! @brief 替换会话对象
    //!
    //! @param ori 原始会话对象
    //! @param ptr 替身
    //! @return 替身, nullptr 若出错
    HandlerPtr TcpAppServer::ReplaceHandler(HandlerPtr const & ori, HandlerPtr const & ptr)
    {
        assert(nullptr != ori && nullptr != ptr);
        assert(mHandlers[ori->mIdx] == ori);

        int idx = ori->mIdx;
        ptr->mWakeUpper = std::move(ori->mWakeUpper);

        ptr->mIdx = idx;
        mHandlers[idx] = ptr;
        return ptr;
    }

}
}
