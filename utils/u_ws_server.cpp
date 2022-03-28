/******************************************************************************
 * 
 * 非阻塞WebSocketServer - web socket 服务器例程
 * 
 * 单线程
 * 
 *****************************************************************************/
#include <XiaoTuNetBox/WebSocketServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <glog/logging.h>

using namespace std::placeholders;
using namespace xiaotu::net;

float test_data[4] = {
    3.1415926,
    1.41421,
    2.71828,
    0.618
};

//! 注意多线程
void OnNewMsg(WebSocketSessionPtr const & session, WebSocketMsgPtr const & msg)
{
    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
    std::cout << msg->mPayload.size() << std::endl;

    RawMsgPtr sendmsg = BuildWsRawMsg(EWsOpcode::eWS_OPCODE_BINARY, (uint8_t*)test_data, 4 * sizeof(float));
    session->SendRawMsg(sendmsg);
}

int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_log_dir = "/home/gyc/logs";

    PollLoopPtr loop = CreatePollLoop();
    ThreadWorkerPtr worker(new ThreadWorker);
    WebSocketServer ws(loop, 65530, 300);

    ws.mWorkSpace = "/home/gyc/tmp";
    ws.SetWorker(worker);
    ws.SetMsgCallBk(std::bind(&OnNewMsg, _1, _2));

    loop->Loop(1000);
    return 0;
}





