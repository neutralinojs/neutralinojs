#include "log.h"
#include "../functions.h"

std::mutex log::_mutex;
std::atomic<bool> log::_log_to_file (true);
std::atomic<bool> log::_log_to_stdout (true);

log::log() :  _lock_guard (_mutex)
{
    if (_log_to_file) {
        _log_file.open(functions::getAppPath() + "/neutralinojs.log", std::ios::app);
    }
}

bool log::isLogToLogFileEnabled()
{
    return _log_to_file;
}

bool log::isLogToStdoutEnabled()
{
    return _log_to_stdout;
}

void log::setLogToLogFileEnabled(const bool enabled)
{
    _log_to_file = enabled;
}

void log::setLogToStdoutEnabled(const bool enabled)
{
    _log_to_stdout = enabled;
}
