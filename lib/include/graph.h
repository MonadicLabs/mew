#pragma once

#include <map>

#include "mew.h"
#include "node.h"
#include "connection.h"

namespace mew
{
    class Graph : public Node
    {
    public:
        Graph( Mew* context )
            :Node(context)
        {

        }

        virtual ~Graph()
        {

        }

    private:
        std::map< uint16_t, Node* >         _nodes;
        std::map< uint16_t, Connection* >   _connections;

    protected:

    };
}
