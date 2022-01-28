#ifndef XTNB_DATA_ARRAY_HPP
#define XTNB_DATA_ARRAY_HPP

#include <iostream>
#include <cassert>

namespace xiaotu {
namespace net {

    template <class T>
    class DataArray {
        public:
            /*
             * 默认构造函数
             */
            DataArray() : mStorBegin(0), mStorEnd(0), mEnd(0) { }
            /*
             * 构造函数
             *
             * @size: 构建数组大小
             */
            DataArray(int size) {
                int c = 4;
                while (c < size)
                    c *= 2;
    
                mStorBegin = new T[c];
                mStorEnd = mStorBegin + c;
                mEnd = mStorBegin + size;
            }
    
            DataArray(int size, T const & v) {
                int c = 4;
                while (c < size)
                    c *= 2;
    
                mStorBegin = new T[c];
                mStorEnd = mStorBegin + c;
                mEnd = mStorBegin + size;
    
                for (int i = 0; i < size; i++)
                    mStorBegin[i] = v;
            }
            /*
             * 构造函数
             *
             * @buf: 初值元素缓存
             * @len: 数组大小
             */
            DataArray(T const *buf, int len) {
                int c = 1;
                while (c < len)
                    c *= 2;
    
                mStorBegin = new T[c];
                mStorEnd = mStorBegin + c;
                mEnd = mStorBegin + len;
    
                for (int i = 0; i < len; i++)
                    mStorBegin[i] = buf[i];
            }
            /*
             * 拷贝构造函数
             */
            DataArray(DataArray const & array) {
                int c = array.Capacity();
                int n = array.Size();
                
                mStorBegin = new T[c];
                mStorEnd = mStorBegin + c;
                mEnd = mStorBegin + n;
    
                for (int i = 0; i < n; i++)
                    mStorBegin[i] = array[i];
            }
            /*
             * 析构函数
             */
            ~DataArray() {
                if (0 != mStorBegin)
                    delete [] mStorBegin;
            }
    
        public:
            int Capacity() const { return mStorEnd - mStorBegin; }
            int Size() const { return mEnd - mStorBegin; }
            int Available() const { return mStorEnd - mEnd; }
            bool Empty() const { return mEnd == mStorBegin; }
            bool Full() const { return mStorEnd == mEnd; }
    
        public:
            /*
             * ajust_capacity - 调整数组容量
             *
             * @c: 容量
             */
            int AdjustCapacity(int c) {
                int n = Size();
                if (c < n)
                    return 1;
    
                if (c == Capacity())
                    return 0;
    
                int capacity = c > 0 ? c : 1;
                T *tmp = new T[capacity];
                for (int i = 0; i < n; i++)
                    tmp[i] = mStorBegin[i];
    
                delete [] mStorBegin;
                mStorBegin = tmp;
                mStorEnd = mStorBegin + capacity;
                mEnd = mStorBegin + n;
    
                return 0;
            }
            /*
             * Resize - 调整数组大小
             */
            int Resize(int size) {
                int cap = this->Capacity();
                if (0 == cap)
                    cap = 1;
                while (cap < size)
                    cap *= 2;
    
                int err = AdjustCapacity(cap);
                if (0 != err)
                    return err;
    
                mEnd = mStorBegin + size;
                return 0;
            }
            /*
             * Clear - 清空数组
             */
            void Clear() {
                mEnd = mStorBegin;
            }
            /*
             * Find - 查找元素返回指针
             */
            T* Find(T const & e) const {
                for (T *p = mStorBegin; p < mEnd; p++) {
                    if (e == *p)
                        return p;
                }
            
                return 0;
            }
            /*
             * FindIdx - 查找元素返回索引
             */
            int FindIdx(T const & e) const {
                int n = Size();
            
                for (int i = 0; i < n; i++)
                    if (e == mStorBegin[i])
                        return i;
            
                return -1;
            }
            /*
             * Has - 判定是否有某个元素
             */
            bool Has(T const & e) const {
                return (0 != Find(e));
            }
            /*
             * Insert - 插入元素
             *
             * @i: 插入目标位置
             * @e: 目标元素
             */
            int Insert(int i, T const & e) {
                int n = Size();
                if (i > n)
                    return 1;
    
                int err = Resize(n + 1);
                if (0 != err)
                    return err;
    
                n = Size();
                if (i < n) {
                    for (int idx = n; idx >= i; idx--)
                        mStorBegin[idx] = mStorBegin[idx-1];
                }
                mStorBegin[i] = e;
                return 0;
            }
            /*
             * Insert -向数组中的第i个位置上插入len个元素
             *
             * @i: 目标索引[0,n]
             * @buf: 元素缓存 
             * @len: 元素数量
             */
            int Insert(int i, T const *buf, int len) {
                int n = Size();
                if (i > n)
                    return 1;
            
                int err = Resize(n + len);
                if (0 != err)
                    return err;
            
                if (i < n) {
                    for (int idx = n; idx >= i; idx--)
                        mStorBegin[idx + len - 1] = mStorBegin[idx-1];
                }
    
                for (int idx = 0; idx < len; idx++)
                    mStorBegin[idx + i] = buf[idx];
                return 0;
            }
            /*
             * Remove - 移除一段数据[from, to)
             *
             * @from: 移除段起始索引,从0开始
             * @to: 移除段结束索引,从0开始
             */
            int Remove(int from, int to) {
                if (to < from)
                    return 1;
            
                int n = Size() + 1;
                if (to >= n)
                    return 2;
    
                int len = n - to;
                for (int idx = 0; idx < len; idx++)
                    mStorBegin[from + idx] = mStorBegin[to + idx];
     
                mEnd -= (to - from);
                return 0;
            }
            /*
             * Remove - 移除一个数据
             *
             * @id: 移除的目标数据索引
             */
            int Remove(int id) {
                return Remove(id, id+1);
            }
    
            int Swap(int i, int j) {
                int n = Size();
                if (i >= n || j >= n)
                    return 1;
    
                T tmp = mStorBegin[i];
                mStorBegin[i] = mStorBegin[j];
                mStorBegin[j] = tmp;
                return 0;
            }
    
            int Push(T const & e) {
                if (Full()) {
                    int ncap = this->Capacity() * 2;
                    if (ncap == 0) ncap = 1;
                    int err = AdjustCapacity(ncap);
                    if (0 != err)
                        return err;
                }
                mEnd[0] = e;
                mEnd++;
                return 0;
            }

            int Push(T const * buf, int n) {
                int s = Size();
                int c = Capacity();
                int ns = n + s;
                if (ns > c) {
                    int err = AdjustCapacity(ns * 2);
                    if (0 != err)
                        return err;
                }

                for (int i = 0; i < n; i++)
                    mEnd[i] = buf[i];
                mEnd += n;
                return 0;
            }
    
            int Pop(T & buf) {
                if (Empty())
                    return 1;
                mEnd--;
                buf = mEnd[0];
                return 0;
            }
    
        public:
            T const & operator [] (int idx) const {
                assert((mStorBegin + idx) < mStorEnd);
                return mStorBegin[idx];
            }
    
            T & operator [] (int idx) {
                assert((mStorBegin + idx) < mStorEnd);
                return mStorBegin[idx];
            }
    
            DataArray & operator = (DataArray const &array) {
                int n = array.Size();
    
                if (this->Capacity() < n) {
                    delete [] mStorBegin;
    
                    int c = array.Capacity();
                    mStorBegin = new T[c];
                    mStorEnd = mStorBegin + c;
                }
    
                for (int i = 0; i < n; i++)
                    mStorBegin[i] = array[i];
                mEnd = mStorBegin + n;
    
                return *this;
            }
    
            friend std::ostream & operator << (std::ostream & stream, DataArray const & array) {
                stream << "[";
    
                int n = array.Size();
                if (n > 0)
                    stream << array[0];
    
                for (int i = 1; i < n; i++)
                    stream << ", " << array[i];
    
                stream << "]";
    
                return stream;
            }
    
            DataArray & operator << (T const & e) {
                Push(e);
                return *this;
            }
    
            DataArray & operator , (T const & e) {
                Push(e);
                return *this;
            }
    
            T * Data() { return mStorBegin; }
    
        private:
            T *mStorBegin;
            T *mStorEnd;
            T *mEnd;
    };
}
}

#endif

