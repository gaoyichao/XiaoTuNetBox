#include <XiaoTuNetBox/InBufObserver.h>

#include <functional>
#include <unistd.h>
#include <cassert>

#include <sys/uio.h>


namespace xiaotu {
namespace net {
 
    size_t InputBuffer::Read(int md)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        int iovcnt = 3;
        struct iovec vec[3];
        uint8_t extrabuf[1024];

        vec[0].iov_base = mReadBuf.Empty() ? mReadBuf.GetStorBeginAddr() : mReadBuf.GetEndAddr();
        vec[0].iov_len = mReadBuf.FreeTail();
        vec[1].iov_base = mReadBuf.GetStorBeginAddr();
        vec[1].iov_len = mReadBuf.FreeHead();
        vec[2].iov_base = extrabuf;
        vec[2].iov_len = 1024;

        size_t n = readv(md, vec, iovcnt);
        if (n > 0) {
            int ava = mReadBuf.Available();
            if (n > ava) {
                mReadBuf.AcceptBack(ava);
                mReadBuf.PushBack(extrabuf, n - ava);
            } else {
                mReadBuf.AcceptBack(n);
            }
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


    void InputBuffer::ObserverCallBack()
    {
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


