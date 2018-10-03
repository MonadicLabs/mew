#pragma once

#include "wsserver_reqproc.h"
#include "wsserver_respcrafter.h"

#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
typedef websocketpp::server<websocketpp::config::asio> ws_server;

namespace mew
{
    class Server;
    class WebSocketServer
    {
    public:
        WebSocketServer( mew::Server* mewServer )
            :_mewServer(mewServer)
        {
            init();
        }

        virtual ~WebSocketServer()
        {

        }

        void start();
        void processMessage( websocketpp::connection_hdl hdl, const std::string& msg );

        ws_server * ws()
        {
            return _wsServer;
        }

        bool addResponseCrafter( std::string reqType, std::shared_ptr< WebSocketServerResponseCrafter > rc );

    private:
        void init();

    protected:
        mew::Server * _mewServer;
        ws_server * _wsServer;
        WebSocketServerRequestProcessor _reqproc;

    };

}
