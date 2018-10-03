#pragma once

#include "wsmessage.h"

#include <memory>

namespace mew
{
    class WebSocketServerResponseCrafter
    {
    public:
        WebSocketServerResponseCrafter()
        {

        }

        virtual ~WebSocketServerResponseCrafter()
        {

        }

        virtual std::shared_ptr< Message > craft( std::shared_ptr< Message > request, void* user_data ) = 0;

    private:

    protected:

    };
}
