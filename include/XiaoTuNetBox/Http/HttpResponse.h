/************************************************************************************
 * 
 * HttpRequest 
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_RESPONSE_H
#define XTNB_HTTP_RESPONSE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>

#include <stdio.h>
#include <sys/stat.h>

namespace xiaotu {
namespace net {

    class HttpResponse {
        public:
            //! @brief 响应报文状态码
            enum EStatusCode {
                eUnknown = 0,
                e101_SwitchProtocol = 101,
                e200_OK = 200,
                e400_BadRequest = 400,
                e404_NotFound = 404,
                e503_ServiceUnavilable = 503
            };
            //! @brief 响应报文状态码解释字符串 map
            static std::map<EStatusCode, std::string> mEStatusCodeToStringMap;

        public:
            //! @brief 构造函数
            //! @param close 发送完该响应报文之后是否期望关闭连接
            HttpResponse(bool close = true) { Reset(close); }

            ~HttpResponse()
            {
                if (NULL != mFilePtr)
                    fclose(mFilePtr);
            }

            //! @brief 设定响应报文的状态码
            inline void SetStatusCode(EStatusCode code)
            {
                assert(mHeadEndIdx < 0);
                mStatusCode = code;
            }
            //! @brief 获取响应报文的状态码
            inline EStatusCode GetStatusCode() const { return mStatusCode; }
            //! @brief 获取响应报文状态的解释字符串
            inline std::string const & GetStatusCodeStr() const
            {
                auto it = mEStatusCodeToStringMap.find(mStatusCode);
                if (mEStatusCodeToStringMap.end() == it)
                    return mEStatusCodeToStringMap[eUnknown];
                return it->second;
            }
            
            //! @brief 设定响应报文首部的键值对
            void SetHeader(std::string const & key, std::string const & value)
            {
                assert(mHeadEndIdx < 0);
                mHeaders[key] = value;
            }
            //! @brief 获取键值对列表
            std::map<std::string, std::string> const & GetHeader() const { return mHeaders; }

            //! @brief 标记关闭连接
            void SetClosing(bool close) { mCloseConnection = close; }

            //! @brief 判定报文是否期望关闭连接
            bool CloseConnection()
            {
                return mCloseConnection || (e200_OK != mStatusCode &&
                                            e101_SwitchProtocol != mStatusCode);
            }

        public:
            //! @brief 获取报文的起始行
            std::string StartLine();

            //! @brief 序列化所有报文
            //! @param buf 记录报文的缓存
            void ToUint8Vector(std::vector<uint8_t> & buf);

        private:
            //! @brief 响应报文的状态码
            EStatusCode mStatusCode;
            //! @brief 响应报文首部的键值对
            std::map<std::string, std::string> mHeaders;
            //! @brief 发送该报文之后是否希望关闭连接
            bool mCloseConnection;

        public:
            inline void AppendContent(uint8_t const * buf, uint32_t n)
            {
                assert(mHeadEndIdx >= 0);
                mContent.insert(mContent.end(), buf, buf + n);
            }

            inline void AppendContent(std::string const & content)
            {
                AppendContent((uint8_t const *)content.data(), content.size());
            }

            inline void AppendContent(std::vector<uint8_t> const & content)
            {
                AppendContent(content.data(), content.size());
            }

            //! @brief 将 cstr 字符串添加到 mContent 中。
            //! 不安全，不推荐使用
            void AppendContent(char const * buf);
        public:
            //! @brief 重置报文，包括状态码、首部键值对、正文
            //! @param close 是否期望关闭连接
            void Reset(bool close = true);

            //! @brief 生成首部数据，并加锁
            //! @param size 正文长度
            void LockHead(size_t size);

            //! @brief 复位 mHeadEndIdx，该操作将导致 mContent 被清空
            void UnlockHead();

            //! @brief 获取报文内容
            std::vector<uint8_t> const & GetContent() { return mContent; }

            int GetHeadEndIdx() { return mHeadEndIdx; }
            //! @brief 获取剩余正文长度
            size_t GetDataSize() { return mDataSize; }
        private:
            //! @brief 报文首部结尾索引
            //! mHeadEndIdx < 0，可以正常修改状态码和首部，此时 mContent 应当为空
            //! mHeadEndIdx >= 0，不可以修改状态码和首部，需手动调用 UnlockHead 复位
            int mHeadEndIdx;

            //! @brief 剩余正文长度
            size_t mDataSize;

            //! @brief 响应报文的具体数据，分为首部和正文两个部分。
            //! 区间 [0, mHeadEndIdx) 为首部，区间 [mHeadEndIdx, end) 为正文
            std::vector<uint8_t> mContent;

        public:
            //! @brief 读取文件，将数据放置在 mContent[mHeadEndIdx, end) 中，
            //! 应当在 LockHead 之后调用
            //!
            //! @param len 待加载的数据长度
            void LoadContent(int len);

            //! @brief 记录目标文件名，应当在 LockHead 之前调用
            bool SetFile(std::string const & fname);

            //! @brief 获取文件状态
            struct stat const & GetFileStat() { return mFileStat; }

            int GetLoadCount() { return mLoadCount; }

            //! @brief 目标文件路径
            std::string mFilePath;

        private:
            //! @brief 目标文件的状态
            struct stat mFileStat;

            //! @brief 目标文件
            FILE * mFilePtr;

            //! @brief 加载文件次数
            int mLoadCount;
    };
    typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

}
}

#endif
