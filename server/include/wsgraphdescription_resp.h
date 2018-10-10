#pragma once

#include <iostream>
using namespace std;

#include "wsserver_respcrafter.h"
#include "mewserver.h"
#include "graph.h"

namespace mew
{
    class GraphDescriptionResponse : public WebSocketServerResponseCrafter
    {
    public:
        GraphDescriptionResponse()
        {

        }

        virtual ~GraphDescriptionResponse()
        {

        }

        virtual std::shared_ptr< Message > craft(std::shared_ptr<Message> request, void* user_data )
        {
            mew::Server * mserver = (mew::Server*)user_data;
            Json::object plobjreq = request->getPayload();
            json11::Json jgid = plobjreq[ "id" ];
            std::string str_id = "";
            if( jgid.is_string() )
            {
                str_id = jgid.string_value();
            }

            Json::object reppl;
            if( str_id.size() )
            {
                reppl[ "status" ] = "ok";
                Graph* g = mserver->workspace()->getGraph( str_id );
                Json::array nodeArray;
                Json::array connArray;
                for( Node* n : g->nodes() )
                {
                    nodeArray.push_back( n->str_id() );
                }
                reppl[ "nodes" ] = nodeArray;
                for( Connection* c : g->connections() )
                {
                    connArray.push_back( c->str_id() );
                }
                reppl[ "connections" ] = nodeArray;
            }
            else
            {
                reppl[ "status" ] = "nok";
            }
            std::shared_ptr< Message > response = std::make_shared<Message>();
            response->type() = "graphdesc_response";
            response->setPayload( reppl );
            return response;
        }

    };
}
