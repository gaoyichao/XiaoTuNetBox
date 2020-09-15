#include <gtest/gtest.h>
#include <iostream>

#include <functional>

#include <XiaoTuNetBox/Timer.h>

using namespace xiaotu::net;

void OnTimeOut(TimerPtr const & timer) {
    std::cout << __FUNCTION__ << std::endl;
}

int main(int argc, char *argv[]) {
    PollLoopPtr loop = CreatePollLoop();
    TimerPtr timer = TimerPtr(new Timer());

    struct timespec t;
    t.tv_sec = 1;
    t.tv_nsec = 0;
    timer->RunEvery(t, std::bind(OnTimeOut, timer));

    ApplyOnLoop(timer, loop);

    loop->Loop(10000);

    return 0;
}

