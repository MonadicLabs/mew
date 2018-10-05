#pragma once

#include <string>
#include <ostream>

namespace mew
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

#ifdef USE_LOGGING
#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>
#define NOVALINK_LOG(x) LOG_S(x)
#define NOVALINK_DEBUG(x) DLOG_S(x)
#define NOVALINK_LOG_SCOPE(x) LOG_SCOPE_FUNCTION(x)
#else
#define MEW_LOG(x) mew::nullstr
#define MEW_DEBUG(x) cerr
#define MEW_LOG_SCOPE(x)
#endif

}
