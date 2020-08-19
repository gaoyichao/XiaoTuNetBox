#include <XiaoTuNetBox/ConnectionNode.h>
#include <iostream>


namespace xiaotu {
namespace net {

    ConnectionNode::ConnectionNode() {
        prev = this;
        next = this;
    }

    void Add(ConnectionNode * node, ConnectionNode * prev, ConnectionNode *next) {
        next->prev = node;
        node->next = next;
        node->prev = prev;
        prev->next = node;
    }

    void AddHead(ConnectionNode * node, ConnectionNode * head) {
        Add(node, head, head->next);
    }

    void AddTail(ConnectionNode * node, ConnectionNode * head) {
        Add(node, head->prev, head);
    }


    void Delete(ConnectionNode * prev, ConnectionNode * next) {
        next->prev = prev;
        prev->next = next;
    }

    void Delete(ConnectionNode * node) {
        Delete(node->prev, node->next);
    }


}
}

