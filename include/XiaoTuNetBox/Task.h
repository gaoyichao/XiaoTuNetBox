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
 
    typedef std::function< bool ()> TaskFunc;

    class Task {
        public:
            Task(TaskFunc func)
            {
                success = false;
                mFunction = std::move(func);
            }

            void SetSuccessFunc(TaskFunc func) { OnSuccess = std::move(func); }
            void SetFailureFunc(TaskFunc func) { OnFailure = std::move(func); }

            void Finish() {
                success = mFunction();
            }

            void Success() {
                OnSuccess();
            }

            void Failure() {
                OnFailure();
            }

            bool success;

            TaskFunc OnSuccess;
            TaskFunc OnFailure;
        private:
            TaskFunc mFunction;
    };
    typedef std::shared_ptr<Task> TaskPtr;


}
}

#endif
