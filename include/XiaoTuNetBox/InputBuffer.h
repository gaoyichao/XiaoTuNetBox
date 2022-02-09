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
            inline size_t ReadableBytes() const { return mWriteIdx - mReadIdx; }
            inline size_t WritableBytes() const { return mReadBuf.Size() - mWriteIdx; }
            inline size_t PrependableBytes() const { return mReadIdx; }

            inline size_t Size() { return ReadableBytes(); }

            inline uint8_t const * Begin()
            {
                return mReadBuf.Data() + mReadIdx;
            }

            inline uint8_t const * End()
            {
                return mReadBuf.Data() + mWriteIdx;
            }

            inline uint8_t const * Peek()
            {
                return Begin();
            }

            inline bool PeekFront(uint8_t *buf, int n, size_t offset = 0)
            {
                assert(n <= (ReadableBytes() - offset));
                memcpy(buf, mReadBuf.Data() + offset, n); 
                return true;
            }

            inline bool DropAll()
            {
                mReadIdx = 0;
                mWriteIdx = 0;
                return true;
            }

            inline bool DropFront(int n)
            {
                int nread = ReadableBytes();
                assert(n <= nread);

                if (n == nread)
                    return DropAll();
                mReadIdx += n;
                return true;
            }
        private:
            DataArray<uint8_t> mReadBuf;
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
                mObservers[idx].reset();
                mObsHoles.push_back(idx);
            }

            inline size_t ObserverSize() const { return mObservers.size(); }
            inline size_t NumHoles() const { return mObsHoles.size(); }
            inline size_t NumObservers() const { return ObserverSize() - NumHoles(); }

        private:
            std::vector<InBufObserverPtr> mObservers;
            std::vector<size_t> mObsHoles;
    };
}
}

#endif
