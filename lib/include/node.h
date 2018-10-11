#pragma once

#include "port.h"
#include "identifiable.h"
#include "parameterizable.h"
#include "abstractregister.h"

#include <iostream>
using namespace std;

namespace mew
{
    class Node;
    typedef AbstractFactory<Node> NodeFactory;

    class Graph;
    class WorkSpace;
    class Node : public EntityIdentifiable, public Parameterizable
    {

    public:
        Node( WorkSpace* ctx = nullptr, Graph* parent = nullptr )
            :_context(ctx), _parent(parent), _tickTimerRef(0), _tickInterval(0.0)
        {

        }

        virtual ~Node()
        {

        }

        virtual void tick( double dt )
        {
            onTimer(dt);
        }

        Graph* parent()
        {
            return _parent;
        }

        Port* in( const std::string& portName )
        {
            return _inputPorts[ portName ];
        }

        Port* out( const std::string& portName )
        {
            return _outputPorts[ portName ];
        }

        // Events
        virtual void onTimer( double dt )
        {

        }

        virtual void onInput( const std::string& portName, cpp17::any& v )
        {
            cerr << "default onInput()" << endl;
        }
        //

        WorkSpace* context()
        {
            return _context;
        }

        void setContext( WorkSpace* ctx )
        {
            _context = ctx;
            onContextChange( _context );
        }

        void setParent( Graph* parent )
        {
            _parent = parent;
        }

        // Node registration
        static void registerNode( const std::string& nodeType, std::shared_ptr<NodeFactory> factory );
        static Node* create( const std::string& nodeType );
        //

    protected:
        mew::WorkSpace * _context;

    protected:
        // Ticking stuff...
        void setTickInterval( double intervalSeconds );
        double _tickInterval;
        void * _tickTimerRef;

        // Ports
        std::map< std::string, Port* > _inputPorts;
        std::map< std::string, Port* > _outputPorts;
        void declare_input(const std::string& portName, int queueSize = 10 );

        void declare_output( const std::string& portName );

        void processInput( const std::string& inputPortName, cpp::any& value )
        {
            onInput( inputPortName, value );
        }

        virtual bool declare_parameter( const std::string &paramName, Value::Type type, Value defaultValue );

        virtual void onContextChange( WorkSpace* ctx )
        {

        }

    public:
        // Utils
        std::string getInputPortFullPath( const std::string& inputPortName )
        {
            std::string nodeProquint = this->str_id();
            std::string ret = inputPortName + "@" + nodeProquint;
            return ret;
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

    private:
        // Node registrar
        static AbstractRegister<Node>& _nodeRegistrar();
        //

    };
}

#define REGISTER_NODE(klass) \
    class klass##Factory : public mew::NodeFactory { \
    public: \
    klass##Factory() \
    { \
    std::shared_ptr<klass##Factory> myself(this); \
    mew::Node::registerNode(#klass, myself); \
    } \
    virtual mew::Node* create() { \
    return new mew::klass(); \
    } \
    }; \
    static klass##Factory global_##klass##Factory;
