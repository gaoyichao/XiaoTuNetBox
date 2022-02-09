#ifndef XTNB_INBUF_OBSERVER_H
#define XTNB_INBUF_OBSERVER_H

#include <XiaoTuNetBox/InputBuffer.h>
#include <XiaoTuNetBox/Utils.h>

#include <memory>
#include <functional>
#include <cassert>

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
            inline size_t Size()
            {
                return mBuffer.ReadableBytes() - mStartIdx;
            }

            inline uint8_t const * Peek()
            {
                return mBuffer.Peek() + mStartIdx;
            }

            inline uint8_t const * Begin()
            {
                return Peek();
            }

            inline uint8_t const * End()
            {
                return mBuffer.End();
            }

            inline bool Empty() { return Begin() == End(); }

            //! @brief 窥视缓存数据，找到第一个满足 pattern 的地址
            //!
            //! @param pattern 模板起始地址
            //! @param np 模板长度
            //! @return 第一个匹配的起始地址, 如果没有找到返回NULL
            inline uint8_t const * PeekFor(uint8_t const * pattern, int np)
            {
                return FindString(Begin(), End(), pattern, np);
            }

            inline bool PeekFront(uint8_t * buf, int n)
            {
                assert(n <= (mBuffer.ReadableBytes() - mStartIdx));
                memcpy(buf, Begin(), n); 
                return true;
            }

            inline bool PopFront(uint8_t *buf, int n)
            {
                assert(n <= (mBuffer.ReadableBytes() - mStartIdx));
                memcpy(buf, Begin(), n); 
                mStartIdx += n;
                return true;
            }

            inline bool DropFront(int n)
            {
                assert(n <= (mBuffer.ReadableBytes() - mStartIdx));
                mStartIdx += n;
                return true;
            }

            inline bool DropAll()
            {
                mStartIdx += Size();
                return true;
            }

            inline void Release()
            {
                mBuffer.ReleaseObserver(mIdx);
            }

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
