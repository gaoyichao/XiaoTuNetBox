#ifndef XTNB_INBUF_OBSERVER_H
#define XTNB_INBUF_OBSERVER_H

#include <XiaoTuNetBox/InputBuffer.h>

#include <memory>
#include <functional>

namespace xiaotu {
namespace net {

    class InBufObserver {
        friend class InputBuffer;
        private:
            InBufObserver(InputBuffer & buffer, size_t idx)
                : mBuffer(buffer), mStartIdx(0), mIdx(idx)
            {

            }
        public:
            inline int Size()
            {
                size_t idx = 0;
                {
                    std::lock_guard<std::mutex> lock(mBuffer.mBufMutex);
                    idx = mStartIdx;
                }
                return mBuffer.Size() - idx;
            }

            inline bool PeekFront(uint8_t * buf, int n)
            {
                size_t idx = 0;
                {
                    std::lock_guard<std::mutex> lock(mBuffer.mBufMutex);
                    idx = mStartIdx;
                }
                return mBuffer.PeekFront(buf, n, idx);
            }

            inline bool PopFront(uint8_t *buf, int n)
            {
                size_t idx = 0;
                {
                    std::lock_guard<std::mutex> lock(mBuffer.mBufMutex);
                    idx = mStartIdx;
                }
                bool suc = mBuffer.PeekFront(buf, n, idx);

                std::lock_guard<std::mutex> lock(mBuffer.mBufMutex);
                if (suc)
                    mStartIdx += n;
                return suc;
            }

            inline bool DropFront(int n)
            {
                assert(n < Size());

                std::lock_guard<std::mutex> lock(mBuffer.mBufMutex);
                mStartIdx += n;
            }

            inline void Release()
            {
                mBuffer.ReleaseObserver(mIdx);
            }

        public:
            typedef std::function<void()> EventCallBk;
            void SetRecvCallBk(EventCallBk cb) { mRecvCallBk = std::move(cb); }
        private:
            EventCallBk mRecvCallBk;

        private:
            InputBuffer & mBuffer;
            //! 有效缓存数据的起始索引
            size_t mStartIdx;

            //! 在 #mBuffer 的观察者列表中的索引位置
            size_t mIdx;
    };

    typedef std::shared_ptr<InBufObserver> InBufObserverPtr;
    typedef std::weak_ptr<InBufObserver> InBufObserverWeakPtr;

}
}

 #endif