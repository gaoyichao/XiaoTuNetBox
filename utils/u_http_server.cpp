/******************************************************************************
 * 
 * u_http_server - http服务器例程
 * 
 * 单线程
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/Event.h>
#include <XiaoTuNetBox/Http/HttpServer.h>
#include <XiaoTuNetBox/ThreadWorker.h>
#include <XiaoTuNetBox/Utils.h>

#include <glog/logging.h>

using namespace std::placeholders;
using namespace xiaotu::net;


int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_log_dir = "~/logs";

    PollLoopPtr loop = Create<PollLoop>();
    ThreadWorkerPtr worker(new ThreadWorker);
    HttpServer http(loop, 65530, 3);

    http.mWorkSpace = ".";
    http.SetWorker(worker);

    loop->Loop(10);

    return 0;
}


