#ifndef XTNB_DATAQUEUE_H
#define XTNB_DATAQUEUE_H

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>

namespace xiaotu {
namespace net {

    template <class T>
    class DataQueue {
        public:
            /*
             * 默认构造函数,初始容量为4
             */
            DataQueue()
            {
                T *tmp = (T*)malloc(4 * sizeof(T));
                assert(NULL != tmp);

                RecordMem(tmp, 4, 0);
            }
            /*
             * 析构函数
             */
            ~DataQueue()
            {
                if (0 != mStorBegin)
                    free(mStorBegin);
            }

        public:
            /*
             * Capacity - 获取队列容量
             */
            int Capacity() const { return mStorEnd - mStorBegin; }

            /*
             * Size - 获取队列中已存数据长度
             */
            int Size() const
            {
                if (0 == mEnd)
                    return 0;
                if (mBegin < mEnd)
                    return mEnd - mBegin;
                else
                    return (mStorEnd - mBegin) + (mEnd - mStorBegin);
            }

            /*
             * Available - 获取队列中可用容量
             */
            int Available() const { return Capacity() - Size(); }
            /*
             * Folded - 判定队列是否折叠
             */
            bool Folded() const
            {
                if (Empty())
                    return false;

                if (Full())
                    return mBegin > mStorBegin;
                return mEnd < mBegin;
            }
            /*
             * FoldedHead - 获取折叠之后，头部的长度
             */
            int FoldedHead() const
            {
                if (!Folded())
                    return 0;
                return mStorEnd - mBegin;
            }
            /*
             * FoldedTail - 获取折叠之后，尾部的长度
             */
            int FoldedTail() const
            {
                if (!Folded())
                    return 0;
                return mEnd - mStorBegin;
            }

            /*
             * FreeTail - 获取队尾空闲空间大小
             */
            int FreeTail() const
            {
                if (Empty())
                    return Capacity();

                if (Folded())
                    return mBegin - mEnd;
                else
                    return mStorEnd - mEnd;
            }
            /*
             * FreeHead - 获取队首空闲空间大小
             */
            int FreeHead() const {
                if (Folded())
                    return 0;
                return mBegin - mStorBegin;
            }
            /*
             * Empty - 判定队列是否为空
             */
            bool Empty() const { return (0 == mEnd); }
            bool Empty()
            {
                if (0 == mEnd) {
                    mBegin = mStorBegin;
                    return true;
                }
                return false;
            }

            /*
             * Full - 判定队列是否满
             */
            bool Full() const { return mEnd == mBegin; }
        public:
            /*
             * Clear - 清空队列
             */
            void Clear()
            {
                mBegin = mStorBegin;
                mEnd = 0;
            }
            /*
             * Rearrange - 整理队列
             * 
             * mStorBegin == mBegin < mEnd < mStorEnd 
             * 保持队列的容量和大小不变
             */
            void Rearrange()
            {
                if (mStorBegin == mBegin)
                    return;
                Realloc(Capacity());
            }
            /*
             * AdjustCapacity - 调整队列容量
             * 
             * 新容量一定能够装下目前队列中所有的有效数据
             *
             * @c: 容量
             */
            bool AdjustCapacity(int c)
            {
                int s = Size();
                if (c < s) {
                    Rearrange();
                    return false;
                }

                if (c == Capacity()) {
                    Rearrange();
                    return true;
                }

                Realloc(c);
                return true;
            }

            /*
             * PushFront - 対首插入数据 size = size + 1
             * 
             * @e: 目标数据
             */
            bool PushFront(T const & e)
            {
                if (Full())
                    AdjustCapacity(2 * Capacity());

                if (Empty())
                    mEnd = mBegin;

                mBegin--;
                if (mBegin < mStorBegin)
                    mBegin = mStorEnd - 1;
                mBegin[0] = e;

                return true;
            }
            /*
             * PopFront - 从対首获取数据 size = size - 1
             */
            bool PopFront(T & buf)
            {
                if (Empty())
                    return false;

                buf = mBegin[0];
                mBegin++;

                if (mBegin == mStorEnd)
                    mBegin = mStorBegin;
                if (mBegin == mEnd) {
                    mBegin = mStorBegin;
                    mEnd = 0;
                }

                return true;
            }
            /*
             * PopFront - 从対首获取数据 size = size - n
             * 
             * @buf: 输出数据缓存
             * @n: 获取数量
             */
            bool PopFront(T *buf, int n)
            {
                assert(n > 0 && n <= Size());
                if (Empty())
                    return false;

                if (mBegin >= mEnd) {
                    int s1 = mStorEnd - mBegin;
                    s1 = (s1 < n) ? s1 : n;
                    memcpy(buf, mBegin, s1 * sizeof(T));

                    n -= s1;
                    buf += s1;
                    mBegin += s1;
                    if (mBegin == mStorEnd)
                        mBegin = mStorBegin;
                }

                if (n > 0) {
                    int s2 = mEnd - mBegin;
                    assert(s2 >= n);
                    memcpy(buf, mBegin, n * sizeof(T));
                    mBegin += n;

                    if (mBegin == mEnd) {
                        mBegin = mStorBegin;
                        mEnd = 0;
                    }
                }

                return true;
            }
            /*
             * DropFront - 抛弃队首的数据 size = size - n
             * 
             * @n: 数量
             */
            bool DropFront(int n) {
                if (0 == n)
                    return true;

                assert(n > 0 && n <= Size());

                if (Empty())
                    return false;

                if (mBegin >= mEnd) {
                    int s1 = mStorEnd - mBegin;
                    s1 = (s1 < n) ? s1 : n;

                    n -= s1;
                    mBegin += s1;
                    if (mBegin == mStorEnd)
                        mBegin = mStorBegin;
                }

                if (n > 0) {
                    int s2 = mEnd - mBegin;
                    assert(s2 >= n);
                    mBegin += n;

                    if (mBegin == mEnd) {
                        mBegin = mStorBegin;
                        mEnd = 0;
                    }
                }

                return true;
            }

            /*
             * AcceptBack - 接受队尾之后的数据 size = size + n
             * 
             * @n: 数量
             */
            bool AcceptBack(int n) {
                if (0 == n)
                    return true;

                assert(n > 0 && n <= Available());

                if (Full())
                    return false;

                if (Empty())
                    mEnd = mBegin;

                if (mEnd >= mBegin) {
                    int s1 = mStorEnd - mEnd;
                    s1 = (s1 < n) ? s1 : n;

                    n -= s1;
                    mEnd += s1;
                    if (mEnd == mStorEnd)
                        mEnd = mStorBegin;
                }

                if (n > 0) {
                    int s2 = mBegin - mEnd;
                    assert(s2 >= n);
                    mEnd += n;
                }

                return true;
            }
            /*
             * PushBack - 队尾插入数据 size = size + 1
             * 
             * @e: 目标数据
             */
            bool PushBack(T const & e)
            {
                if (Full())
                    AdjustCapacity(2 * Capacity());

                if (Empty())
                    mEnd = mBegin;

                mEnd[0] = e;
                mEnd++;
                if (mEnd == mStorEnd)
                    mEnd = mStorBegin;

                return true;
            }
            /*
             * PushBack - 队尾插入数据 size = size + n
             * 
             * @buf: 目标数据缓存
             * @n: 插入数量
             */
            bool PushBack(T const *buf, int n)
            {
                assert(n > 0);

                int c = Capacity();
                int s = Size();

                if ((s + n) > c)
                    AdjustCapacity(2 * (s + n));

                if (Empty())
                    mEnd = mBegin;

                if (mBegin <= mEnd) {
                    int s1 = mStorEnd - mEnd;
                    s1 = (s1 < n) ? s1 : n;
                    memcpy(mEnd, buf, s1 * sizeof(T));
                    n -= s1;
                    buf += s1;
                    mEnd += s1;
                    if (mEnd == mStorEnd)
                        mEnd = mStorBegin;
                }

                if (n > 0) {
                    int s2 = mBegin - mEnd;
                    assert(s2 > n);
                    memcpy(mEnd, buf, n * sizeof(T));
                    mEnd += n;
                }
                return true;
            }
            /*
             * PopBack - 从队尾获取数据 size = size - 1
             */
            bool PopBack(T & buf)
            {
                if (Empty())
                    return false;

                mEnd--;
                if (mEnd < mStorBegin)
                    mEnd = mStorEnd - 1;

                buf = mEnd[0];

                if (mBegin == mEnd) {
                    mBegin = mStorBegin;
                    mEnd = 0;
                }

                return true;
            }
            /*
             * PeekFront - 窥视対首数据 size = size
             */
            bool PeekFront(T & buf)
            {
                if (Empty())
                    return false;

                buf = mBegin[0];

                return true;
            }
            /*
             * PeekBack - 窥视队尾数据 size = size
             */
            bool PeekBack(T & buf)
            {
                if (Empty())
                    return false;

                T *tmp = mEnd - 1;
                if (tmp < mStorBegin)
                    tmp = mStorEnd - 1;

                buf = *tmp;
                return true;
            }

            /*
             * Enqueue - 进队，插入数据到队尾
             */
            bool Enqueue(T const & e) { return PushBack(e); }
            /*
             * Dequeue - 出队，从対首取出数据
             */
            bool Dequeue(T & buf) { return PopPront(buf); }

        protected:
            /*
             * RecordMem - 记录新申请的内存
             * 
             * 每次新申请内存就必须保证mStorBegin == mBegin < mEnd < mStorEnd
             * 
             * @new_mem: 新申请的内存地址
             * @c: 容量
             * @s: 有效数据长度
             */
            void RecordMem(T * new_mem, int c, int s)
            {
                mStorBegin = new_mem;
                mStorEnd = mStorBegin + c;
                mBegin = mStorBegin;
                mEnd = (0 == s) ? NULL : (mStorBegin + s);
            }
            /*
             * Realloc - 重新申请内存并拷贝数据
             * 
             * @c: 容量
             */
            T * Realloc(int c)
            {
                int s = Size();
                T *tmp = (T*)malloc(c * sizeof(T));

                if (!Empty()) {
                    if (mBegin < mEnd) {
                        memcpy(tmp, mBegin, s * sizeof(T));
                    } else {
                        int s1 = mStorEnd - mBegin;
                        memcpy(tmp, mBegin, s1 * sizeof(T));
                        memcpy(tmp + s1, mStorBegin, (unsigned)(s - s1) * sizeof(T));
                    }
                }

                free(mStorBegin);
                RecordMem(tmp, c, s);
                return tmp;
            }

        public:
            T const * GetStorBeginAddr() const { return mStorBegin; }
            T const * GetStorEndAddr() const { return mStorEnd; }
            T const * GetBeginAddr() const { return mBegin; }
            T const * GetEndAddr() const { return mEnd; }

            T * GetStorBeginAddr() { return mStorBegin; }
            T * GetStorEndAddr() { return mStorEnd; }
            T * GetBeginAddr() { return mBegin; }
            T * GetEndAddr() { return mEnd; }

        protected:
            T *mStorBegin;
            T *mStorEnd;
            T *mBegin;
            T *mEnd;
    };

}
}


#endif

