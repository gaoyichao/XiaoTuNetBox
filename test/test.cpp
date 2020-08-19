#include <XiaoTuNetBox/Timer.h>
#include <XiaoTuNetBox/PollLoop.h>

#include <iostream>
#include <functional>
#include <memory>
    
using namespace xiaotu::net;

void CallBack() {
    std::cout << __FUNCTION__ << std::endl;
}

int main(int argc, char *argv[])
{
    std::shared_ptr<Timer> time = std::shared_ptr<Timer>(new Timer());
    PollLoopPtr loop = CreatePollLoop();

    struct timespec t;
    t.tv_sec = 1;
    t.tv_nsec = 0;
    time->RunEvery(t, std::bind(&CallBack));

    ApplyOnLoop(time, loop);

    loop->Loop(1000);

    return 0;
}



