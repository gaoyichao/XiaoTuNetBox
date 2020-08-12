#include <XiaoTuNetBox/ThreadTools.h>

#include <sys/syscall.h>

namespace xiaotu {
namespace net {


    pid_t ThreadTools::GetCurrentTid() {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

}
}


