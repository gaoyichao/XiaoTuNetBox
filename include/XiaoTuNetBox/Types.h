#ifndef XTNB_TYPES_H
#define XTNB_TYPES_H

#include <vector>
#include <memory>

namespace xiaotu {
namespace net {

    typedef std::vector<uint8_t> RawMsg;
    typedef std::shared_ptr<RawMsg> RawMsgPtr;
    typedef std::shared_ptr<const RawMsg> RawMsgConstPtr;


}
}

#endif

