#ifndef XTNB_INPUT_BUFFER_H
#define XTNB_INPUT_BUFFER_H

#include <XiaoTuNetBox/Types.h>
#include <XiaoTuNetBox/DataQueue.hpp>

#include <mutex>
#include <memory>
#include <vector>

namespace xiaotu {
namespace net {
 
    class InBufObserver;

    class InputBuffer {
        public:
            InputBuffer();
            ~InputBuffer();
            /*
             * Read - 读文件描述符中的数据
             */
            size_t Read(int md);

            inline int Size()
            {
                int re = 0;
                {
                    std::lock_guard<std::mutex> lock(mBufMutex);
                    re = mReadBuf.Size();
                }
                return re;
            }

            inline bool PeekFront(uint8_t *buf, int n, size_t offset = 0)
            {
                bool re = false;
                {
                    std::lock_guard<std::mutex> lock(mBufMutex);
                    re = mReadBuf.PeekFront(buf, n, offset);
                }
                return re;
            }

            inline bool DropFront(int n)
            {
                bool re = false;
                {
                    std::lock_guard<std::mutex> lock(mBufMutex);
                    re = mReadBuf.DropFront(n);
                }
                return re;
            }

        private:
            int Size(InBufObserver & obs);
            bool PeekFront(uint8_t * buf, int n, InBufObserver & obs);
            bool PopFront(uint8_t * buf, int n, InBufObserver & obs);
            bool DropFront(int n, InBufObserver & obs);
            
        private:
            size_t mExtraBufSize;
            uint8_t * mExtraBuf;
            DataQueue<uint8_t> mReadBuf;
            std::mutex mBufMutex;

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
