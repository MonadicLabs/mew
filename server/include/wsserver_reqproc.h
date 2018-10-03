#pragma once

#include "wsmessage.h"
#include "wsserver_respcrafter.h"

#include <memory>
#include <map>

namespace mew
{
    class WebSocketServerRequestProcessor
    {
    public:
        WebSocketServerRequestProcessor()
        {

        }

        virtual ~WebSocketServerRequestProcessor()
        {

        }

        std::shared_ptr< Message > process(std::shared_ptr< Message > input , void * user_data );
        bool addResponseCrafter( std::string reqType, std::shared_ptr< WebSocketServerResponseCrafter > crafter );

    private:
        std::map< std::string, std::shared_ptr< WebSocketServerResponseCrafter > > _responses;

    protected:

    };
}
