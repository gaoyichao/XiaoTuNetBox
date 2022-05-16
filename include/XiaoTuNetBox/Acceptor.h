#ifndef XTNB_ACCEPTOR_H
#define XTNB_ACCEPTOR_H

#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/PollEventHandler.h>

namespace xiaotu {
namespace net {

    class Acceptor;
    typedef std::shared_ptr<Acceptor> AcceptorPtr;
    typedef std::shared_ptr<const Acceptor> AcceptorConstPtr;
    AcceptorPtr CreateAcceptor(int port, int qsize);

    class Acceptor {
        public:
            Acceptor(Acceptor const &) = delete;
            Acceptor & operator = (Acceptor const &) = delete;

            PollEventHandlerPtr & GetHandler() { return mEventHandler; }
            Socket & GetSocket() { return mAccpSock; }
            void OnReadEvent();

        friend AcceptorPtr CreateAcceptor(int port, int qsize);
        private:
            Acceptor(int port, int qsize);

        private:
            Socket mAccpSock;
            PollEventHandlerPtr mEventHandler;

        public:
            typedef std::function<void(int, IPv4Ptr const &)> NewConnCallBk;
            void SetNewConnCallBk(NewConnCallBk cb) { mNewConnCallBk = std::move(cb); }

        private:
            NewConnCallBk mNewConnCallBk;
    };
}
}





#endif


