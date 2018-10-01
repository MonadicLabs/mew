#pragma once

namespace mew
{

}

/* in H
#pragma once

#include <string>
#include <ostream>

namespace novadem
{
namespace link
{
void log_init( int argc, char** argv, const std::string& logFilePath );
class NullStream : public std::ostream {
    class NullBuffer : public std::streambuf {
    public:
        int overflow( int c ) { return c; }
    } m_nb;
public:
    NullStream() : std::ostream( &m_nb ) {}
};
extern NullStream nullstr;
}
}

#ifdef USE_LOGGING
#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>
#define NOVALINK_LOG(x) LOG_S(x)
#define NOVALINK_DEBUG(x) DLOG_S(x)
#define NOVALINK_LOG_SCOPE(x) LOG_SCOPE_FUNCTION(x)
#else
#define NOVALINK_LOG(x) novadem::link::nullstr
#define NOVALINK_DEBUG(x) cerr
#define NOVALINK_LOG_SCOPE(x)
#endif
*/

/* in CPP
#ifdef USE_LOGGING
#define LOGURU_IMPLEMENTATION 1
#endif

#include "log.h"

novadem::link::NullStream novadem::link::nullstr;
void novadem::link::log_init(int argc, char **argv, const std::string &logFilePath)
{
#ifdef USE_LOGGING
    loguru::init(argc, argv);
    loguru::add_file( logFilePath.c_str(), loguru::Append, loguru::Verbosity_MAX);
#else

#endif
}
*/
