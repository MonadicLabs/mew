
#include "wsserver.h"
#include "wsmessage.h"
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef ws_server::message_ptr message_ptr;

// CRAFTERS
#include "wshello_resp.h"
#include "wslistgraph_resp.h"
#include "wscreategraph_resp.h"
#include "wsgraphdescription_resp.h"
//

#include <iostream>
using namespace std;

// Define a callback to handle incoming messages
void on_message(mew::WebSocketServer* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->ws()->stop_listening();
        return;
    }

    try {

        // s->ws()->send( hdl, "coucouc !", msg->get_opcode() );
        s->processMessage( hdl, msg->get_payload() );

    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

void mew::WebSocketServer::start()
{
    // Create a server endpoint
    _wsServer = new ws_server();

    try {
        // Set logging settings
        _wsServer->set_access_channels(websocketpp::log::alevel::all);
        _wsServer->clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        _wsServer->init_asio();

        // Register our message handler
        _wsServer->set_message_handler(bind(&on_message,this,::_1,::_2));

        // Listen on port 9002
        _wsServer->listen(9002);

        // Start the server accept loop
        _wsServer->start_accept();

        // Start the ASIO io_service run loop
        _wsServer->run();

    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }

}

void mew::WebSocketServer::processMessage(websocketpp::connection_hdl hdl, const std::string &msg)
{
    cerr << "processMessage" << endl;
    std::shared_ptr< mew::Message > req = mew::Message::fromPayload( msg );

    // Craft response
    std::shared_ptr< mew::Message > response = _reqproc.process( req, _mewServer );

    // Send
    if( !response )
    {
        response = mew::Message::craftErrorMessage( 0, "invalid request" );
    }
    _wsServer->send( hdl, response->toString(), websocketpp::frame::opcode::TEXT );
}

bool mew::WebSocketServer::addResponseCrafter(string reqType, std::shared_ptr<mew::WebSocketServerResponseCrafter> rc)
{
    return _reqproc.addResponseCrafter( reqType, rc );
}

void mew::WebSocketServer::init()
{
    _reqproc.addResponseCrafter( "hello", std::make_shared< HelloResponse >() );
    _reqproc.addResponseCrafter( "listgraph", std::make_shared< ListGraphResponse >() );
    _reqproc.addResponseCrafter( "creategraph", std::make_shared< CreateGraphResponse >() );
    _reqproc.addResponseCrafter( "graphdesc", std::make_shared< GraphDescriptionResponse >() );
}
