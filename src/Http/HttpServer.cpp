#include <XiaoTuNetBox/Http/HttpServer.h>
#include <XiaoTuNetBox/Utils.h>

#include <XiaoTuNetBox/Http/HttpModuleCheckURL.h>
#include <XiaoTuNetBox/Http/HttpModuleIdentify.h>
#include <XiaoTuNetBox/Http/HttpModuleLogin.h>
#include <XiaoTuNetBox/Http/HttpModuleLoadFile.h>

#include <cassert>
#include <functional>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glog/logging.h>


namespace xiaotu {
namespace net {

    using namespace std::placeholders;

    HttpServer::HttpServer(EventLoopPtr const & loop, int port,
                           int max_conn, std::string const & ws)
        : TcpAppServer(loop, port, max_conn, ws)
    {
        //! @todo 事件轮盘配置化
        mServer->SetTimeOut(10, 0, 5);
        mServer->SetNewConnCallBk(std::bind(&HttpServer::OnNewConnection, this, _1));
        mServer->SetCloseConnCallBk(std::bind(&HttpServer::OnCloseConnection, this, _1));
        mServer->SetMessageCallBk(std::bind(&HttpServer::OnMessage, this, _1, _2, _3));

        //! @todo 根据配置文件构建 Http 处理模块, mWorkSpace
        HttpModulePtr invalidRequestModule = std::make_shared<HttpModuleInvalidRequest>();
        HttpModulePtr parseCookieModule    = std::make_shared<HttpModuleParseCookie>();
        HttpModulePtr identifyModule       = std::make_shared<HttpModuleCookieIdentify>();
        HttpModuleLoginPtr loginModule     = std::make_shared<HttpModuleLogin>();
        HttpModulePtr checkURLModule       = std::make_shared<HttpModuleCheckURL>(mWorkSpace);
        HttpModuleCheckMethodPtr checkMethodModule = std::make_shared<HttpModuleCheckMethod>();
        HttpModulePtr unSupportModule      = std::make_shared<HttpModuleUnSupport>();
        HttpModulePtr loadFileModule       = std::make_shared<HttpModuleLoadFile>();
        HttpModulePtr responseModule       = std::make_shared<HttpModuleResponse>();

        invalidRequestModule->SetSuccessModule(parseCookieModule);
        invalidRequestModule->SetFailureModule(responseModule);

        parseCookieModule->SetSuccessModule(identifyModule);

        identifyModule->SetSuccessModule(checkURLModule);
        identifyModule->SetFailureModule(loginModule);

        loginModule->mLoginHtml = "/login.html";
        loginModule->SetSuccessModule(checkURLModule);
        loginModule->SetFailureModule(responseModule);

        checkURLModule->SetSuccessModule(checkMethodModule);
        checkURLModule->SetFailureModule(responseModule);

        checkMethodModule->mOnGetModule = loadFileModule;
        checkMethodModule->mOnPostModule = loadFileModule;
        checkMethodModule->SetFailureModule(unSupportModule);

        loadFileModule->SetFailureModule(responseModule);
        unSupportModule->SetSuccessModule(responseModule);

        mFirstModule = invalidRequestModule;
    }
 
    //! @brief 新建连接，与 EventLoop 在同一个线程中
    //!
    //! @param conn 连接对象
    void HttpServer::OnNewConnection(ConnectionPtr const & conn)
    {
        LOG(INFO) << "新建连接:" << conn->GetInfo();
        conn->GetHandler()->SetNonBlock(true);

        HttpHandlerPtr ptr(new HttpHandler(conn));
        ptr->BuildWakeUpper(mServer->GetLoop(),
                std::bind(&HttpServer::OnTaskFinished, ConnectionWeakPtr(conn)));

        conn->mUserObject = ptr;
        AddHandler(ptr);
    }

    //! @brief 解析消息，与 EventLoop 在同一个线程中，当成功解析处请求报文之后，
    //! 根据请求方法构建对应的处理任务
    //!
    //! @param con tcp连接
    //! @param buf 接收消息缓存
    //! @param n 接收消息字节数
    void HttpServer::OnMessage(ConnectionPtr const & con, uint8_t const * buf, ssize_t n)
    {
        LOG(INFO) << "接收到了消息";
        HttpHandlerPtr ptr = std::static_pointer_cast<HttpHandler>(con->mUserObject.lock());

        uint8_t const * begin = buf;
        uint8_t const * end = buf + n;

        while (begin < end) {
            HttpRequestPtr req = ptr->GetRequest();
            begin = req->Parse(begin, end);

            if (HttpRequest::eResponsing == req->GetState() || HttpRequest::eError == req->GetState()) {
                mFirstModule->Handle(ptr);
            }

            if (nullptr == begin)
                break;
        }
    }

    // loop 线程
    void HttpServer::OnCloseConnection(ConnectionPtr const & conn) {
        LOG(INFO) << "关闭连接:" << conn->GetInfo();
        HttpHandlerPtr ptr = std::static_pointer_cast<HttpHandler>(conn->mUserObject.lock());
        ReleaseHandler(ptr);
    }

    /*************************************************************************************************/

    //! @brief 任务完成之后的回调函数，与 EventLoop 在同一个线程中
    void HttpServer::OnTaskFinished(ConnectionWeakPtr const & conptr)
    {
        ConnectionPtr con = conptr.lock();
        if (nullptr == con)
            return;
        HttpHandlerPtr session = std::static_pointer_cast<HttpHandler>(con->mUserObject.lock());
        if (nullptr == session)
            return;

        if (con->IsClosed()) {
            session->Reset();
            return;
        }

        TaskPtr const & currTask = session->mCurrTask;
        if (currTask->success) {
            if (currTask->OnSuccess)
                currTask->Success();
            else
                HttpServer::OnTaskSuccessDefault(con, session);
        } else {
            if (currTask->OnFailure)
                currTask->Failure();
            else
                HttpServer::OnTaskFailureDefault(con, session);
        }
    }

    //! @brief 任务成功，一般处理方法，与 EventLoop 在同一个线程中
    bool HttpServer::OnTaskSuccessDefault(ConnectionPtr const & con, HttpHandlerPtr const & session)
    {
        HttpResponsePtr res = session->GetResponse();

        std::vector<uint8_t> const & buf = res->GetContent();
        con->SendBytes(buf.data(), buf.size());
        if (res->CloseConnection())
            con->Close();

        session->Reset();
        return true;
    }

    //! @brief 任务失败，一般处理方法，与 EventLoop 在同一个线程中
    bool HttpServer::OnTaskFailureDefault(ConnectionPtr const & con, HttpHandlerPtr const & session)
    {
        con->Close();
        session->Reset();
        return true;
    }
}
}


