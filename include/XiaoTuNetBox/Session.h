/************************************************************************************
 * 
 * Session
 * 
 ***********************************************************************************/
#ifndef XTNB_SESSION_H
#define XTNB_SESSION_H

#include <memory>

namespace xiaotu {
namespace net {

    class Session {
        public:
            Session(Session const &) = delete;
            Session & operator = (Session const &) = delete;

            virtual char const * ToCString() = 0;
        protected:
            Session() = default;
            ~Session() = default;
    };

    typedef std::shared_ptr<Session> SessionPtr;
    typedef std::weak_ptr<Session> SessionWeakPtr;

}
}


#endif
