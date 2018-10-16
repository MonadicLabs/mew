
#include <node.h>

namespace mew {

class Clock : public Node
{
public:
    Clock( WorkSpace* ctx = nullptr );
    virtual ~Clock();

    virtual void onContextChange(WorkSpace *ctx);

    virtual void onTimer(double dt);

    virtual void onSetParameter(const string &paramName, Value value);

private:
    int _cpt;

protected:

};

}
