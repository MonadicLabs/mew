#pragma once

#include <iostream>
using namespace std;

#include "wsserver_respcrafter.h"
#include "mewserver.h"

namespace mew
{
    class HelloResponse : public WebSocketServerResponseCrafter
    {
    public:
        HelloResponse()
        {

        }

        virtual ~HelloResponse()
        {

        }

        virtual std::shared_ptr< Message > craft(std::shared_ptr<Message> request, void* user_data )
        {
            mew::Server * mserver = (mew::Server*)user_data;
            cerr << "mew_server=" << mserver << endl;
            cerr << "HelloResponse !" << endl;
            std::shared_ptr< Message > response = std::make_shared<Message>();
            response->type() = "hello_response";
            return response;
        }

    };
}
