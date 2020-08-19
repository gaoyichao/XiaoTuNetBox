#ifndef XTNB_CONNECTIONNODE_H
#define XTNB_CONNECTIONNODE_H

#include <XiaoTuNetBox/Connection.h>

namespace xiaotu {
namespace net {
 

    class ConnectionNode {
        public:
            ConnectionNode();
            ConnectionNode(ConnectionNode const & node) {
                conn = node.conn;
                prev = this;
                next = this;
            }

            ConnectionNode & operator = (ConnectionNode const & node) {
                conn = node.conn;
                prev = this;
                next = this;
                return *this;
            }


        public:
            ConnectionPtr conn;
            ConnectionNode *prev;
            ConnectionNode *next;
    };

    typedef std::shared_ptr<ConnectionNode> ConnectionNodePtr;
    typedef std::shared_ptr<const ConnectionNode> ConnectionNodeConstPtr;


    void Add(ConnectionNode * node, ConnectionNode * prev, ConnectionNode *next);
    void AddHead(ConnectionNode * node, ConnectionNode * head);
    void AddTail(ConnectionNode * node, ConnectionNode * head);

    void Delete(ConnectionNode * prev, ConnectionNode * next);
    void Delete(ConnectionNode * node);

}
}

#endif

