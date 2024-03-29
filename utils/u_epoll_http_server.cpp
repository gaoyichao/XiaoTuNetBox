/******************************************************************************
 * 
 * u_http_server - http服务器例程
 * 
 * 单线程
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/Event.h>
#include <XiaoTuNetBox/Http/HttpServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <functional>
#include <iostream>
#include <string>
#include <map>
#include <thread>

#include <endian.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std::placeholders;
using namespace xiaotu::net;


int main() {
    EPollLoopPtr loop = Create<EPollLoop>();

    HttpServer http(loop, 65530, 30, ".");
    //http.mWorkSpace = ".";

    loop->Loop(-1);

    return 0;
}


