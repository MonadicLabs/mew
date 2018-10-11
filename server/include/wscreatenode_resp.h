#pragma once

#include <iostream>
using namespace std;

#include "wsserver_respcrafter.h"
#include "mewserver.h"
#include "graph.h"

namespace mew
{
class CreateNodeResponse : public WebSocketServerResponseCrafter
{
public:
    CreateNodeResponse()
    {

    }

    virtual ~CreateNodeResponse()
    {

    }


    virtual std::shared_ptr< Message > craft(std::shared_ptr<Message> request, void* user_data )
    {
        mew::Server * mserver = (mew::Server*)user_data;

        std::shared_ptr< Message > response = std::make_shared<Message>();
        response->type() = "createnode_response";

        // First, we must retrieve the targeted graph...
        std::string graphId = request->getPayload()[ "graph" ].string_value();
        cerr << "GRAPHID=" << graphId << endl;
        if( graphId.size() )
        {
            mew::Graph* g = mserver->workspace()->getGraph( graphId );
            if( g )
            {
                std::string nodeType = request->getPayload()[ "node_type" ].string_value();
                cerr << "NODE_TYPE=" << nodeType << endl;
                Node * n = Node::create( nodeType );
                cerr << "NODE CREATION RETURNED: " << n << endl;
                if( n != nullptr )
                {
                    n->setContext( mserver->workspace() );
                    n->setParent( g );
                    EntityIdentifiable::ValueType nid = g->addNode( n );
                    Json::object plobject;
                    plobject[ "status" ] = "ok";
                    plobject[ "node_id" ] = ((EntityIdentifiable*)n)->str_id();
                    response->setPayload( plobject );
                }
                else
                {
                    response = mew::Message::craftErrorMessage( 0, "invalid node type" );
                }
            }
            else
            {
                response = mew::Message::craftErrorMessage( 0, "invalid graph id" );
            }
        }
        return response;
    }

};
}
