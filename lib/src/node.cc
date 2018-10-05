
#include "node.h"
#include "mew.h"
#include "workspace.h"
#include "inputport.h"
#include "outputport.h"

void mew::Node::setTickInterval(double intervalSeconds)
{
    if( intervalSeconds > 0.0 )
    {
        if( _tickTimerRef == 0 )
        {
            std::function<void(mew::Mew*,double)> f = [this](mew::Mew*, double dt){
                this->tick( dt );
            };
            _tickTimerRef = _context->_runtime->timer( f, intervalSeconds );
        }
        else
        {
            _context->_runtime->set_timer_interval( _tickTimerRef, intervalSeconds );
        }
    }
}

void mew::Node::declare_input(const string &portName, int queueSize )
{
    std::function<void(mew::Mew*, cpp::any&)> f = [portName, this](mew::Mew*, cpp::any& v){
        this->processInput( portName, v );
    };
    // Request subscription here ! TODO.
    // cerr << "port path = " << getInputPortFullPath( portName ) << endl;
    //            void * ipSubCtx = _context->subscribe( getInputPortFullPath( portName ), f );
    InputPort * ip = new InputPort( this, queueSize );
    ip->_label = portName;
    _inputPorts.insert( make_pair( portName, ip ) );
}

void mew::Node::declare_output(const string &portName)
{
    _outputPorts.insert( make_pair( portName, new OutputPort(this) ) );
}
