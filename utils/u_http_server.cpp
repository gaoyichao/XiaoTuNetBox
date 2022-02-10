/******************************************************************************
 * 
 * 非阻塞HttpServer - http服务器例程
 * 
 * 单线程
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/HttpServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <functional>
#include <iostream>
#include <string>
#include <map>
#include <thread>

#include <endian.h>
#include <string.h>

using namespace std::placeholders;
using namespace xiaotu::net;

void OnOkHttpRequest(HttpRequestPtr const & req,
                     HttpResponsePtr const & res)
{
    res->SetStatusCode(HttpResponse::e200_OK);
    res->AppendContent("<h1>Hello</h1>");
}

int main() {
    PollLoopPtr loop = CreatePollLoop();
    HttpServer http(loop, 65530, 3);
    http.SetRequestCallBk(std::bind(&OnOkHttpRequest, _1, _2));
    std::thread t([&]{ loop->Loop(1000); });

    std::shared_ptr<int> douniwan(new int(10));
    std::weak_ptr<int> bienao = douniwan;
    std::cout << "douniwna:" << douniwan.use_count() << std::endl;

    t.join();
    return 0;
}


