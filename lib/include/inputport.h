#pragma once

#include "port.h"
#include "workspace.h"

namespace mew
{
class InputPort : public Port
{
public:
    InputPort( Node* parent, size_t queueSize )
        :Port(parent), _subCtx(0)
    {

    }

    virtual ~InputPort()
    {

    }

    virtual void connectFrom( Port* p )
    {
        cerr << "connect from " << p << endl;
        if( _subCtx != 0 )
        {
            // unsubscribe first ?
        }
        else
        {
            WorkSpace* ws = _parent->context();
            std::function<void(mew::Mew*,cpp17::any)> f = [this](mew::Mew*, cpp17::any v){
                cerr << "LDFKDLKGLDFKGLDGKDLKFGDLKG" << endl;
                _parent->onInput( _label, v );
            };
            std::string portPath = _parent->getInputPortFullPath( _label );
            cerr << "inputPort path=" << portPath << endl;
            _subCtx = ws->getRuntime()->subscribe( portPath, f );
        }
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

    virtual std::string getPubAddress()
    {
        return _parent->getInputPortFullPath( _label );
    }

    void* _subCtx;

protected:
    moodycamel::ConcurrentQueue<cpp17::any> _queue;

};
}
