#include <XiaoTuNetBox/InputBuffer.h>
#include <XiaoTuNetBox/InBufObserver.h>

#include <functional>
#include <unistd.h>
#include <cassert>

#include <sys/uio.h>

namespace xiaotu {
namespace net {

    const size_t InputBuffer::mDefaultPrependSize = 8;
    const size_t InputBuffer::mInitialSize = 1024;
    const size_t InputBuffer::mExtraBufSize = 1024;

    InputBuffer::InputBuffer()
        : mReadBuf(mInitialSize)
    {
        mExtraBuf = new uint8_t[mExtraBufSize];

        mReadIdx = 0;
        mWriteIdx = 0;
    }

    InputBuffer::~InputBuffer()
    {
        delete [] mExtraBuf;
    }


    size_t InputBuffer::Read(int fd)
    {
        std::lock_guard<std::mutex> lock(mBufMutex);
        // 清理队首数据
        size_t min_idx = mReadBuf.Size();
        for (size_t i = 0; i < mObservers.size(); ++i) {
            if (nullptr == mObservers[i])
                continue;
            if (min_idx > mObservers[i]->mStartIdx)
                min_idx = mObservers[i]->mStartIdx;
        }
        mReadIdx += min_idx;
        for (size_t i = 0; i < mObservers.size(); ++i) {
            if (nullptr == mObservers[i])
                continue;
            mObservers[i]->mStartIdx -= min_idx;
        }

        // 搬运内核数据
        int iovcnt = 2;
        int nwrite = WritableBytes();
        struct iovec vec[2];
        vec[0].iov_base = &mReadBuf[mWriteIdx];
        vec[0].iov_len = nwrite;
        vec[1].iov_base = mExtraBuf;
        vec[1].iov_len = mExtraBufSize;
        size_t n = readv(fd, vec, iovcnt);
        if (n > 0) {
            if (n > nwrite) {
                int n_ext = n - nwrite;
                mReadBuf.Push(mExtraBuf, n_ext);
            }
            mWriteIdx += n;
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

    int InputBuffer::Size(InBufObserver & obs)
    {
        std::lock_guard<std::mutex> lock(mBufMutex);
        return (ReadableBytes() - obs.mStartIdx);
    }

    bool InputBuffer::PeekFront(uint8_t * buf, int n, InBufObserver & obs)
    {
        std::lock_guard<std::mutex> lock(mBufMutex);
        assert(n <= (ReadableBytes() - obs.mStartIdx));
        memcpy(buf, mReadBuf.Data() + mReadIdx + obs.mStartIdx, n); 
        return true;
    }

    bool InputBuffer::PopFront(uint8_t * buf, int n, InBufObserver & obs)
    {
        std::lock_guard<std::mutex> lock(mBufMutex);
        assert(n <= (ReadableBytes() - obs.mStartIdx));
        memcpy(buf, mReadBuf.Data() + mReadIdx + obs.mStartIdx, n); 
        obs.mStartIdx += n;
        return true;

    }

    bool InputBuffer::DropFront(int n, InBufObserver & obs)
    {
        std::lock_guard<std::mutex> lock(mBufMutex);
        int idx = mReadIdx + obs.mStartIdx + n;
        if (idx >= mWriteIdx)
            return false;
        obs.mStartIdx += n;
        return true;
    }
}
}


