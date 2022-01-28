/*****************************************************************************************
 * 
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 * 
 ****************************************************************************************/
#ifndef XTNB_INPUT_BUFFER_H
#define XTNB_INPUT_BUFFER_H

#include <XiaoTuNetBox/Types.h>
#include <XiaoTuNetBox/DataArray.hpp>

#include <string.h>
#include <mutex>
#include <memory>
#include <vector>

namespace xiaotu {
namespace net {
 
    class InBufObserver;

    class InputBuffer {
        public:
            static const size_t mDefaultPrependSize;
            static const size_t mInitialSize;
            static const size_t mExtraBufSize;

        public:
            InputBuffer();
            ~InputBuffer();

        public:
            inline size_t Size()
            {
                std::lock_guard<std::mutex> lock(mBufMutex);
                return ReadableBytes();
            }

            inline bool PeekFront(uint8_t *buf, int n, size_t offset = 0)
            {
                std::lock_guard<std::mutex> lock(mBufMutex);
                assert(n <= (ReadableBytes() - offset));
                memcpy(buf, mReadBuf.Data() + offset, n); 
                return true;
            }

            inline bool DropAll()
            {
                std::lock_guard<std::mutex> lock(mBufMutex);
                mReadIdx = 0;
                mWriteIdx = 0;
                return true;
            }

            inline bool DropFront(int n)
            {
                std::lock_guard<std::mutex> lock(mBufMutex);
                int nread = ReadableBytes();
                assert(n <= nread);

                if (n == nread)
                    return DropAll();
                mReadIdx += n;
                return true;
            }
        private:
            int Size(InBufObserver & obs);
            bool PeekFront(uint8_t * buf, int n, InBufObserver & obs);
            bool PopFront(uint8_t * buf, int n, InBufObserver & obs);
            bool DropFront(int n, InBufObserver & obs);
        private:
            inline size_t ReadableBytes() const { return mWriteIdx - mReadIdx; }
            inline size_t WritableBytes() const { return mReadBuf.Size() - mWriteIdx; }
            inline size_t PrependableBytes() const { return mReadIdx; }
           
        private:
            DataArray<uint8_t> mReadBuf;
            std::mutex mBufMutex;
            size_t mReadIdx;
            size_t mWriteIdx;
            uint8_t * mExtraBuf;

        public:
            size_t Read(int fd);

        public:
            friend class InBufObserver;
            typedef std::shared_ptr<InBufObserver> InBufObserverPtr;
            InBufObserverPtr CreateObserver();

            inline void ReleaseObserver(size_t idx)
            {
                std::lock_guard<std::mutex> lock(mObsMutex);
                mObservers[idx].reset();
                mObsHoles.push_back(idx);
            }

            void ObserverCallBack();
            
        private:
            std::mutex mObsMutex;
            std::vector<InBufObserverPtr> mObservers;
            std::vector<size_t> mObsHoles;
    };
}
}

#endif
