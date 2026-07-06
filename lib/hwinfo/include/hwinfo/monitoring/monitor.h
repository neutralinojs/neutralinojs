#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "hwinfo/platform.h"

namespace hwinfo::monitoring {

// Periodically fetches data via FetchFn and delivers it to a Callback on a background thread.
// Call start() to begin and stop() to halt. The destructor calls stop() automatically.
template <typename T>
class Monitor {
 public:
  using Callback = std::function<void(const T&)>;
  using FetchFn = std::function<T()>;

  Monitor(FetchFn fetch, Callback callback, std::chrono::milliseconds interval)
      : _fetch(std::move(fetch)), _callback(std::move(callback)), _interval(interval) {}

  ~Monitor() { stop(); }

  Monitor(const Monitor&) = delete;
  Monitor& operator=(const Monitor&) = delete;
  Monitor(Monitor&&) = delete;
  Monitor& operator=(Monitor&&) = delete;

  void start() {
    if (_running.exchange(true)) return;
    _thread = std::thread([this]() {
      while (_running) {
        _callback(_fetch());
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait_for(lock, _interval, [this]() { return !_running.load(); });
      }
    });
  }

  void stop() {
    _running = false;
    _cv.notify_all();
    if (_thread.joinable()) {
      if (_thread.get_id() == std::this_thread::get_id()) {
        _thread.detach();
      } else {
        _thread.join();
      }
    }
  }

  HWI_NODISCARD bool is_running() const { return _running.load(); }

 private:
  FetchFn _fetch;
  Callback _callback;
  std::chrono::milliseconds _interval;
  std::atomic<bool> _running{false};
  std::mutex _mutex;
  std::condition_variable _cv;
  std::thread _thread;
};

}  // namespace hwinfo::monitoring
