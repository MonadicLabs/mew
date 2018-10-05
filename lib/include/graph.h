#pragma once

#include <cuckoohash_map.hh>

#include "workspace.h"
#include "node.h"
#include "connection.h"

namespace mew
{
    class Graph : public Node
    {
    public:
        Graph( mew::WorkSpace* context, Graph* parent = 0 )
            :Node(context, parent)
        {

        }

        virtual ~Graph()
        {

        }

        EntityIdentifiable::ValueType addNode( mew::Node* n )
        {
            n->_parent = this;
            EntityIdentifiable::ValueType nid = _context->assignId( n );
            _nodes.insert( nid, n );
            return nid;
        }

        EntityIdentifiable::ValueType addConnection( mew::Port* src, mew::Port* dst )
        {
            mew::Connection* conn = new mew::Connection( src, dst );
            EntityIdentifiable::ValueType cid = _context->assignId( conn );
            conn->set_id( cid );
            _connections.insert( make_pair( cid, conn ) );
            return cid;
        }

        std::vector< EntityIdentifiable::ValueType > node_ids()
        {
            std::vector< EntityIdentifiable::ValueType > ret;
            /*
            for( auto& kv : _nodes )
            {
                ret.push_back( kv.first );
            }
            */
            return ret;
        }

    private:
        cuckoohash_map< EntityIdentifiable::ValueType, Node* >         _nodes;
        std::map< EntityIdentifiable::ValueType, Connection* >   _connections;

    protected:

    };
}
