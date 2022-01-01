// Clip Library
// Copyright (c) 2015-2018 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip.h"
#include "clip_lock_impl.h"

#include <cassert>
#include <map>
#include <vector>

namespace clip {

typedef std::vector<char> Buffer;
typedef std::map<format, Buffer> Map;

static format g_last_format = 100; // TODO create an enum with common formats
static Map g_data;

lock::impl::impl(void* native_handle) : m_locked(true) {
}

lock::impl::~impl() {
}

bool lock::impl::clear() {
  g_data.clear();
  return true;
}

bool lock::impl::is_convertible(format f) const {
  return (g_data.find(f) != g_data.end());
}

bool lock::impl::set_data(format f, const char* buf, size_t len) {
  Buffer& dst = g_data[f];

  dst.resize(len);
  if (buf && len > 0)
    std::copy(buf, buf+len, dst.begin());

  return true;
}

bool lock::impl::get_data(format f, char* buf, size_t len) const {
  assert(buf);

  if (!buf || !is_convertible(f))
    return false;

  const Buffer& src = g_data[f];
  std::copy(src.begin(), src.end(), buf);
  return true;
}

size_t lock::impl::get_data_length(format f) const {
  if (is_convertible(f))
    return g_data[f].size();
  else
    return 0;
}

bool lock::impl::set_image(const image& image) {
  return false;               // TODO
}

bool lock::impl::get_image(image& image) const {
  return false;               // TODO
}

bool lock::impl::get_image_spec(image_spec& spec) const {
  return false;               // TODO
}

format register_format(const std::string& name) {
  return g_last_format++;
}

} // namespace clip
