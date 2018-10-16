
#include "console.h"
#include "workspace.h"

#include <iostream>
#include <iomanip>
using namespace std;

mew::Console::Console(WorkSpace *ctx)
    :Node(ctx)
{
    declare_input( "in" );
}

void mew::Console::onInput(const string &portName, any &v)
{
    Value content;
    try{
        content = cpp::any_cast<Value>( v );
        if( content.is(Value::BINARY) )
        {
//            content.
             print_bytes( cerr, "[console]", &content.binary()[0], content.binary().size() );
        }
        else
        {
            cerr << "[console] " << content.str() << endl;
        }
    }
    catch ( cpp::bad_any_cast& e )
    {
        cerr << "CONSOLE BAD template<typename R, typename ...Args>" << endl;
    }
}

void mew::Console::print_bytes(ostream &out, const char *title, const unsigned char *data, size_t dataLen, bool format, int symbol_per_line)
{
    out << title << std::endl;
    out << std::setfill('0');
    for(size_t i = 0; i < dataLen; ++i) {
        out << std::hex << std::setw(2) << (int)data[i];
        if (format) {
            out << (((i + 1) % symbol_per_line == 0) ? "\n" : " ");
        }
    }
    out << std::endl;
}

mew::Console::~Console()
{

}


REGISTER_NODE(Console)
