
#include <node.h>

namespace mew {

class Console : public Node
{
public:
    Console( WorkSpace* ctx = nullptr );
    virtual ~Console();

    virtual void onInput( const std::string& portName, cpp17::any& v );

private:
    void print_bytes(std::ostream& out, const char *title, const unsigned char *data, size_t dataLen, bool format = true, int symbol_per_line = 64 ) ;

protected:

};

}
