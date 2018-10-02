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
        Graph( Mew* context, Graph* parent = 0 )
            :Node(context, parent)
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
