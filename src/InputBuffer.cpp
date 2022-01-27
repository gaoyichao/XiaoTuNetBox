#include <XiaoTuNetBox/InBufObserver.h>

#include <functional>
#include <unistd.h>
#include <cassert>

#include <sys/uio.h>

namespace xiaotu {
namespace net {

    InputBuffer::InputBuffer()
    {
        mExtraBufSize = 1024;
        mExtraBuf = new uint8_t[mExtraBufSize];
    }

    InputBuffer::~InputBuffer()
    {
        delete [] mExtraBuf;
    }
 
    void InputBuffer::DropHead()
    {
        std::lock_guard<std::mutex> lock(mBufMutex);

        size_t min_idx = mReadBuf.Size();
        for (size_t i = 0; i < mObservers.size(); ++i) {
            if (nullptr == mObservers[i])
                continue;
            if (min_idx > mObservers[i]->mStartIdx)
                min_idx = mObservers[i]->mStartIdx;
        }

        mReadBuf.DropFront(min_idx);

        for (size_t i = 0; i < mObservers.size(); ++i) {
            if (nullptr == mObservers[i])
                continue;
            mObservers[i]->mStartIdx -= min_idx;
        }
    }

    size_t InputBuffer::Read(int md)
    {
        DropHead();
        std::lock_guard<std::mutex> lock(mBufMutex);

        int iovcnt = 3;
        struct iovec vec[3];

        vec[0].iov_base = mReadBuf.Empty() ? mReadBuf.GetStorBeginAddr() : mReadBuf.GetEndAddr();
        vec[0].iov_len = mReadBuf.FreeTail();
        vec[1].iov_base = mReadBuf.GetStorBeginAddr();
        vec[1].iov_len = mReadBuf.FreeHead();
        vec[2].iov_base = mExtraBuf;
        vec[2].iov_len = mExtraBufSize;

        size_t n = readv(md, vec, iovcnt);
        if (n > 0) {
            int ava = mReadBuf.Available();
            if (n > ava) {
                mReadBuf.AcceptBack(ava);
                mReadBuf.PushBack(mExtraBuf, n - ava);
            } else {
                mReadBuf.AcceptBack(n);
            }
        }
        return n;
    }

    InBufObserverPtr InputBuffer::CreateObserver()
    {
        std::lock_guard<std::mutex> lock(mObsMutex);
        int idx = 0;
        if (mObsHoles.empty()) {
            idx = mObservers.size();
            mObservers.push_back(nullptr);
        } else {
            idx = mObsHoles.back();
            mObsHoles.pop_back();
            mObservers[idx] = nullptr;
        }

        InBufObserverPtr ptr(new InBufObserver(*this, idx));
        mObservers[idx] = ptr;
        return ptr;
    }


    void InputBuffer::ObserverCallBack()
    {
        std::lock_guard<std::mutex> lock(mObsMutex);
        int n = mObservers.size();
        for (int i = 0; i < n; ++i) {
            if (nullptr == mObservers[i])
                continue;

            if (mObservers[i]->mRecvCallBk)
                mObservers[i]->mRecvCallBk();
        }

    }
 
}
}


