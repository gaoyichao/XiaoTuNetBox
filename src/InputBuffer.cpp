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
        
        if (mWriteIdx == mReadBuf.Capacity()) {
            mReadBuf.AdjustCapacity(2 * mWriteIdx);
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
}
}


