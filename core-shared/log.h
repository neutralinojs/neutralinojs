#ifndef LOG_H
#define LOG_H

#include <atomic>
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
#include <fstream>
#include <memory>
#include <mutex>
#include <iostream>

//! Basic Logging class.
class log {
private:
    static std::mutex _mutex;
    std::lock_guard<std::mutex> _lock_guard;

    // Path of the logfile. This is set during „init“.
    static fs::path _logfile_path;

    // Handle of the logfile.
    std::ofstream _log_file;

    //! If this is true, we log to stdout.
    static std::atomic<bool> _log_to_stdout;

    //! If this is true, we log to a file called „neutralinojs.log“.
    static std::atomic<bool> _log_to_file;

    log();

    log(const log&) = delete;
    log& operator=(const log&) = delete;
    log(log&&) : _lock_guard(_mutex) {}
    log& operator=(log&&) = default;

public:
    ~log() {
        // On destroying the object, we print a newline to stdout and the logfile,
        // but only if either is enabled.
        if (isLogToStdoutEnabled()) {
            std::cout << "\n";
        }

        if (isLogToLogFileEnabled()) {
#ifdef _WIN32
            // For Windows we want to make sure that we got the correct line ending.
            _log_file << "\r";
#endif
            _log_file << "\n";
        }
    }

    template<typename T>
    log& operator <<(const T& val) {
        if (isLogToStdoutEnabled()) {
            std::cout << val;
        }
        if (isLogToLogFileEnabled()) {
            // We use a stringstream to convert val to string. This is more
            // flexible than calling std::to_string.
            std::stringstream ss;
            ss << val;
            _log_file << ss.str();
        }
        return *this;
    }

    static log Log(const std::string& prefix, const std::string& file, const std::string& func) {
        log instance;
        std::stringstream log_str;
        log_str << prefix << " [" + file + ":" + func + "] ";
        if (isLogToStdoutEnabled()) {
            std::cout << log_str.str();
        }
        if (isLogToLogFileEnabled()) {
            instance._log_file << log_str.str();
        }
        return instance;
    }

    //! Enable or disable logging to stdout.
    static void setLogToStdoutEnabled(const bool enabled);

    //! Returns true if logging to stdout is enabled.
    static bool isLogToStdoutEnabled();

    //! Enable or disable logging to neutralinojs.log
    static void setLogToLogFileEnabled(const bool enabled);

    //! Returns true if logging to neutralinojs.log is enabled.
    static bool isLogToLogFileEnabled();
};

#define INFO() log::Log("INFO",__FILE__, __func__)
#define DEBUG() log::Log("DEBUG",__FILE__, __func__)
#define TRACE() log::Log("TRACE",__FILE__, __func__)
#define ERROR() log::Log("ERROR",__FILE__, __func__)
#define WARN() log::Log("WARN",__FILE__, __func__)
#define FIXME() log::Log("FIXME",__FILE__, __func__)

#endif
