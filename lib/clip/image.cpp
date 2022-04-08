// Clip Library
// Copyright (c) 2015-2022 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip.h"

namespace clip {

unsigned long image_spec::required_data_size() const
{
  unsigned long n = (bytes_per_row * height);

  // For 24bpp we add some extra space to access the last pixel (3
  // bytes) as an uint32_t
  if (bits_per_pixel == 24) {
    if ((n % 4) > 0)
      n += 4 - (n % 4);
    else
      ++n;
  }

  return n;
}

image::image()
  : m_own_data(false),
    m_data(nullptr)
{
}

image::image(const image_spec& spec)
  : m_own_data(true),
    m_data(new char[spec.required_data_size()]),
    m_spec(spec) {
}

image::image(const void* data, const image_spec& spec)
  : m_own_data(false),
    m_data((char*)data),
    m_spec(spec) {
}

image::image(const image& image)
  : m_own_data(false),
    m_data(nullptr),
    m_spec(image.m_spec) {
  copy_image(image);
}

image::image(image&& image)
  : m_own_data(false),
    m_data(nullptr) {
  move_image(std::move(image));
}

image::~image() {
  reset();
}

image& image::operator=(const image& image) {
  copy_image(image);
  return *this;
}

image& image::operator=(image&& image) {
  move_image(std::move(image));
  return *this;
}

void image::reset() {
  if (m_own_data) {
    delete[] m_data;
    m_own_data = false;
    m_data = nullptr;
  }
}

void image::copy_image(const image& image) {
  reset();

  m_spec = image.spec();
  std::size_t n = m_spec.required_data_size();

  m_own_data = true;
  m_data = new char[n];
  std::copy(image.data(),
            image.data()+n,
            m_data);
}

void image::move_image(image&& image) {
  std::swap(m_own_data, image.m_own_data);
  std::swap(m_data, image.m_data);
  std::swap(m_spec, image.m_spec);
}

} // namespace clip
