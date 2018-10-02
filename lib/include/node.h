#pragma once

#include "mew.h"
#include "port.h"
#include "identifiable.h"
#include "parameterizable.h"

#include <iostream>
using namespace std;

namespace mew
{
    class Graph;
    class Node : public ShortIdentifiable, public Parameterizable
    {

    public:
        Node( mew::Mew* ctx, Graph* parent = nullptr )
            :_context(ctx), _parent(parent), _tickTimerRef(0), _tickInterval(0.0)
        {
            // setTickInterval( 0.1 );
        }

        virtual ~Node()
        {
            cerr << "NODE_DTOR()" << endl;
            for( auto kv : _inputPorts )
            {
                InputPort * ip = (InputPort*)kv.second;
                if( _context->unsubscribe( ip->_subCtx ) )
                {
                    cerr << "Unsubscribed !!!!" << endl;
                }
                delete kv.second;
            }
            _inputPorts.clear();
            _context->printSubscriptions();
        }

        virtual void tick( double dt )
        {
            cerr << "tick." << endl;
        }

        Graph* parent()
        {
            return _parent;
        }

        virtual void test( Value& v )
        {
            cerr << "test:" << v.str() << endl;
        }

        Port* in( const std::string& portName )
        {
            return _inputPorts[ portName ];
        }

        Port* out( const std::string& portName )
        {
            return _outputPorts[ portName ];
        }

    private:
        mew::Mew * _context;

    protected:
        // Ticking stuff...
        void setTickInterval( double intervalSeconds )
        {
            if( intervalSeconds > 0.0 )
            {
                if( _tickTimerRef == 0 )
                {
                    std::function<void(mew::Mew*,double)> f = [this](mew::Mew*, double dt){
                        this->tick( dt );
                    };
                    _tickTimerRef = _context->timer( f, intervalSeconds );
                }
                else
                {
                    _context->set_timer_interval( _tickTimerRef, intervalSeconds );
                }
            }
        }
        double _tickInterval;
        void * _tickTimerRef;

        // Ports
        std::map< std::string, Port* > _inputPorts;
        std::map< std::string, Port* > _outputPorts;
        void declare_input( const std::string& portName )
        {
            std::function<void(mew::Mew*, Value)> f = [portName, this](mew::Mew*, Value v){
                this->processInput( portName, v );
            };
            cerr << "port path = " << getInputPortFullPath( portName ) << endl;
            void * ipSubCtx = _context->subscribe( getInputPortFullPath( portName ), f );
            InputPort * ip = new InputPort( this, ipSubCtx );
            _inputPorts.insert( make_pair( portName, ip ) );
        }

        void declare_output( const std::string& portName )
        {
            _outputPorts.insert( make_pair( portName, new OutputPort(this) ) );
        }

        void processInput( const std::string& inputPortName, Value& v )
        {
            cerr << "processInput " << endl;
            test( v );
        }

        virtual bool declare_parameter( const std::string &paramName, Value::Type type, Value defaultValue )
        {
            // Add a port for this
            // _inputPorts.insert( std::make_pair( paramName, new ParameterPort( this ) ) );
            Parameterizable::declare_parameter( paramName, type, defaultValue );
        }

    public:
        // Utils
        std::string getInputPortFullPath( const std::string& inputPortName )
        {
            // return getNodeFullPath() + inputPortName;
            return inputPortName;
        }

        std::string getNodeFullPath()
        {
            std::vector< std::string > addresses;
            // cerr << "this_id=" << this->id() << endl;
            Node* curNode = this;
            while( curNode != nullptr )
            {
                std::stringstream sstr;
                sstr << curNode->id();
                addresses.push_back( sstr.str() );
                // cerr << "ptr=" << (int)curNode << " id=" << curNode->id() << endl;
                curNode = (Node*)(curNode->parent());
                // cerr << "next_ptr=" << (int)curNode << endl;
            }
            std::stringstream sstr;
            sstr << "/";
            for( int k = 0; k < addresses.size(); ++k )
            {
                sstr << addresses[k];
                // if( k != addresses.size() - 1 )
                    sstr << "/";
            }
            cerr << "ADDR=" << sstr.str() << endl;
            return sstr.str();
        }

        // hierarchy
        Graph* _parent;

    };
}
