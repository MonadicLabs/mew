
#include "wsserver_reqproc.h"
#include "wsserver_respcrafter.h"

std::shared_ptr<mew::Message> mew::WebSocketServerRequestProcessor::process(std::shared_ptr<mew::Message> input, void* user_data )
{
    std::shared_ptr< Message > ret = nullptr;
    if( _responses.find( input->type() ) != _responses.end() )
    {
        std::shared_ptr< WebSocketServerResponseCrafter > crafter = _responses[ input->type() ];
        ret = crafter->craft( input, user_data );
    }
    else
    {
        ret = mew::Message::craftErrorMessage( 0, "invalid request" );
    }
    return ret;
}

bool mew::WebSocketServerRequestProcessor::addResponseCrafter(std::string reqType, std::shared_ptr<WebSocketServerResponseCrafter> crafter)
{
    if( _responses.find( reqType ) != _responses.end() )
        return false;
    else
        _responses.insert( std::make_pair( reqType, crafter ) );
}
