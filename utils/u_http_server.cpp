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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std::placeholders;
using namespace xiaotu::net;

std::string gRootPath("/home/gyc/tmp");

void OnHeadRequest(HttpRequestPtr const & req,
                   HttpResponsePtr const & res)
{
    res->SetStatusCode(HttpResponse::e200_OK);
}

void OnGetRequest(HttpRequestPtr const & req,
                  HttpResponsePtr const & res)
{
    std::string urlpath = req->GetURLPath();
    if ("/" == urlpath)
        urlpath = "/index.html";

    std::string path = gRootPath + urlpath;
    std::cout << path << std::endl;

    struct stat s;
    if (-1 == stat(path.c_str(), &s)) {
        res->SetStatusCode(HttpResponse::e404_NotFound);
        res->AppendContent("Error:404");
        return;
    }

    if (S_ISDIR(s.st_mode)) {
        res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);
        return;
    }

    if (!S_ISREG(s.st_mode))
        return;
    res->SetStatusCode(HttpResponse::e200_OK);
    res->AppendContent(path, 0, s.st_size);
}


void OnOkHttpRequest(HttpRequestPtr const & req,
                     HttpResponsePtr const & res)
{
    switch (req->GetMethod()) {
        case HttpRequest::eHEAD: {
            OnHeadRequest(req, res);
            break;
        }
        case HttpRequest::eGET: {
            OnGetRequest(req, res);
            break;
        }
        default: {
            res->SetStatusCode(HttpResponse::e503_ServiceUnavilable);
            break;
        }

    }
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


