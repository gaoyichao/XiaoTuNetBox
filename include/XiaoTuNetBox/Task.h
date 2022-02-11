/************************************************************************************
 * 
 * Task 
 * 
 ***********************************************************************************/
#ifndef XTNB_TASK_H
#define XTNB_TASK_H

#include <functional>
#include <memory>

namespace xiaotu {
namespace net {
 
    class Task {
        public:
            typedef std::function< void ()> TaskFunc;

        public:
            Task() {}
            Task(TaskFunc func)
            {
                mFunction = std::move(func);
            }

            void operator () () {
                if (mFunction)
                    mFunction();
            }

            void Finish() {
                if (mFunction)
                    mFunction();
            }

            void SetTaskFunc(TaskFunc func) { mFunction = std::move(func); }
        private:
            TaskFunc mFunction;
    };
    typedef std::shared_ptr<Task> TaskPtr;


}
}

#endif
