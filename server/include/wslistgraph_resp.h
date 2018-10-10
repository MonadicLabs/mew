#pragma once

#include <iostream>
using namespace std;

#include "wsserver_respcrafter.h"
#include "mewserver.h"

namespace mew
{
    class ListGraphResponse : public WebSocketServerResponseCrafter
    {
    public:
        ListGraphResponse()
        {

        }

        virtual ~ListGraphResponse()
        {

        }

        virtual std::shared_ptr< Message > craft(std::shared_ptr<Message> request, void* user_data )
        {
            mew::Server * mserver = (mew::Server*)user_data;
            std::shared_ptr< Message > response = std::make_shared<Message>();
            response->type() = "listgraph_response";
            std::vector< Graph* > glist = mserver->workspace()->listGraphs();
            Json::object plroot;
            Json::array glist_array;
            for( int i = 0; i < glist.size(); ++i )
            {
                EntityIdentifiable* ii = (EntityIdentifiable*)glist[i];
                Json gid_value = ii->str_id();
                glist_array.push_back( gid_value );
            }
            plroot["graphs"] = glist_array;
            response->setPayload( plroot );
            return response;
        }

    };
}
