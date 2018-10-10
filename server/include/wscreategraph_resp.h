#pragma once

#include <iostream>
using namespace std;

#include "wsserver_respcrafter.h"
#include "mewserver.h"

namespace mew
{
    class CreateGraphResponse : public WebSocketServerResponseCrafter
    {
    public:
        CreateGraphResponse()
        {

        }

        virtual ~CreateGraphResponse()
        {

        }


        virtual std::shared_ptr< Message > craft(std::shared_ptr<Message> request, void* user_data )
        {
            mew::Server * mserver = (mew::Server*)user_data;
            mew::Graph* g = mserver->workspace()->createEmptyGraph();
            std::shared_ptr< Message > response = std::make_shared<Message>();
            response->type() = "creategraph_response";
            Json::object plobject;
            if( g != nullptr )
            {
                plobject[ "status" ] = "ok";
                plobject[ "id" ] = ((EntityIdentifiable*)g)->str_id();
            }
            else
            {
                plobject[ "status" ] = "nok";
            }
            response->setPayload( plobject );
            return response;
        }

    };
}
