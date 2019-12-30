#include "log.h"
#ifndef __has_include
  static_assert(false, "__has_include not supported");
#else
#  if __has_include(<filesystem>)
#    include <filesystem>
     namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
     namespace fs = std::experimental::filesystem;
#  endif
#endif

std::mutex log::_mutex;
std::atomic<bool> log::_log_to_file (true);
std::atomic<bool> log::_log_to_stdout (true);
fs::path log::_logfile_path
    (fs::current_path() / "neutralinojs.log");

log::log() :  _lock_guard (_mutex)
{
    if (_log_to_file) {
        _log_file.open("neutralinojs.log", std::ios::app);
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
