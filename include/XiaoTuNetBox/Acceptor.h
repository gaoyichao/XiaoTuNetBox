#ifndef XTNB_ACCEPTOR_H
#define XTNB_ACCEPTOR_H

#include <XiaoTuNetBox/Socket.h>
#include <XiaoTuNetBox/Event/EventLoop.h>
#include <XiaoTuNetBox/Event/EventHandler.h>

namespace xiaotu {
namespace net {

    class Acceptor;
    typedef std::shared_ptr<Acceptor> AcceptorPtr;
    typedef std::shared_ptr<const Acceptor> AcceptorConstPtr;

    AcceptorPtr CreateAcceptor(int port, int qsize, EventLoop const & loop);

    class Acceptor {
        public:
            Acceptor(Acceptor const &) = delete;
            Acceptor & operator = (Acceptor const &) = delete;

            EventHandlerPtr & GetHandler() { return mEventHandler; }
            Socket & GetSocket() { return mAccpSock; }
            void OnReadEvent();

        friend AcceptorPtr CreateAcceptor(int port, int qsize, EventLoop const & loop);
        private:
            Acceptor(int port, int qsize, EventLoop const & loop);

        private:
            Socket mAccpSock;
            EventHandlerPtr mEventHandler;

        public:
            typedef std::function<void(int, IPv4Ptr const &)> NewConnCallBk;
            void SetNewConnCallBk(NewConnCallBk cb) { mNewConnCallBk = std::move(cb); }

        private:
            NewConnCallBk mNewConnCallBk;
    };
}
}





#endif


