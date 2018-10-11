#pragma once

#include "inputport.h"
#include "value.h"

namespace mew
{
class ParameterPort : public InputPort
{
public:
    ParameterPort( Node* parent )
        :InputPort(parent, 1)
    {

    }

    virtual ~ParameterPort()
    {

    }

    virtual std::string connectReceivingEnd( Port* otherEnd )
    {
        if( _subCtx != 0 )
        {
            // unsubscribe first ?
        }
        else
        {
            WorkSpace* ws = _parent->context();
            std::function<void(mew::Mew*,Value)> f = [this](mew::Mew*, Value v){
                _parent->setParameter( _label, v );
            };
            _topic = _parent->getInputPortFullPath( _label );
            cerr << "parameter path=" << _topic << endl;
            _subCtx = ws->getRuntime()->subscribe( _topic, f );
        }
        return _topic;
    }

private:

protected:

};

}
