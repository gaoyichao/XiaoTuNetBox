#ifndef XTNB_ACCEPTOR_H
#define XTNB_ACCEPTOR_H

#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/EventHandler.h>


namespace xiaotu {
namespace net {

    class Acceptor {
        public:
            Acceptor(int port, int qsize);
            PollEventHandlerPtr & GetHandler() { return mEventHandler; }
            Socket & GetSocket() { return mAccpSock; }

            typedef std::function<void(int, IPv4Ptr const &)> NewConnCallBk;
            void SetNewConnCallBk(NewConnCallBk cb) { mNewConnCallBk = std::move(cb); }

            void OnReadEvent();
        private:
            Socket mAccpSock;
            PollEventHandlerPtr mEventHandler;
            NewConnCallBk mNewConnCallBk;
    };
}
}





#endif


