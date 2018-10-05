
#include "log.h"

mew::NullStream mew::nullstr;
void mew::log_init(int argc, char **argv, const std::string &logFilePath)
{
#ifdef USE_LOGGING
    loguru::init(argc, argv);
    loguru::add_file( logFilePath.c_str(), loguru::Append, loguru::Verbosity_MAX);
#else

#endif
}
