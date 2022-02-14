#ifndef XTNB_TYPES_H
#define XTNB_TYPES_H

#include <vector>
#include <memory>

namespace xiaotu {
namespace net {

    typedef std::vector<uint8_t> RawMsg;
    typedef std::shared_ptr<RawMsg> RawMsgPtr;
    typedef std::shared_ptr<const RawMsg> RawMsgConstPtr;

    class Object {
        public:
            virtual char const * ToCString() = 0;
    };
    typedef std::shared_ptr<Object> ObjectPtr;
    typedef std::weak_ptr<Object> ObjectWeakPtr;


}
}

#endif

