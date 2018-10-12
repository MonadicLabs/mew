#pragma once

#include "port.h"
#include "workspace.h"

namespace mew
{
class InputPort : public Port
{
public:
    InputPort( Node* parent, size_t queueSize = 1024 )
        :Port(parent), _subCtx(0)
    {

    }

    virtual ~InputPort()
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
            std::function<void(mew::Mew*,cpp17::any)> f = [this](mew::Mew*, cpp17::any v){
                _parent->onInput( _label, v );
            };
            _topic = _parent->getInputPortFullPath( _label );
            cerr << "inputPort path=" << _topic << endl;
            _subCtx = ws->getRuntime()->subscribe( _topic, f );
        }
        return _topic;
    }

    virtual void disconnectFrom( Port* p )
    {
        cerr << "disconnect from " << p << endl;
    }

    virtual bool read(cpp17::any &v)
    {
        return _queue.try_dequeue(v);
    }

    virtual bool write(cpp17::any v)
    {
        return _queue.try_enqueue(v);
    }

    virtual std::string pub_address()
    {
        return _topic;
    }

    void* _subCtx;

protected:
    moodycamel::ConcurrentQueue<cpp17::any> _queue;
    std::string _topic;

};
}
