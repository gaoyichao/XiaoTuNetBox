/************************************************************************************
 * 
 * HttpTask - 处理 http 请求的任务
 * 
 ***********************************************************************************/
#ifndef XTNB_HTTP_TASK_H
#define XTNB_HTTP_TASK_H

#include <functional>
#include <memory>

namespace xiaotu {
namespace net {
 
    class HttpTask {
        public:
            void operator () () {
                if (mFunction)
                    mFunction();
            }

            void Finish() {
                if (mFunction)
                    mFunction();
            }

        public:
            typedef std::function< void ()> TaskFunc;
            void SetTaskFunc(TaskFunc func) { mFunction = std::move(func); }
        private:
            TaskFunc mFunction;
    };
    typedef std::shared_ptr<HttpTask> HttpTaskPtr;


}
}

#endif
