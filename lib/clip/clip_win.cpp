// Clip Library
// Copyright (C) 2015-2020  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip.h"
#include "clip_common.h"
#include "clip_lock_impl.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <regex>

#include <windows.h>

#if CLIP_ENABLE_IMAGE
#include "clip_win_wic.h"
#endif // CLIP_ENABLE_IMAGE

#ifndef LCS_WINDOWS_COLOR_SPACE
#define LCS_WINDOWS_COLOR_SPACE 'Win '
#endif

#ifndef CF_DIBV5
#define CF_DIBV5            17
#endif

namespace clip {

namespace {

// Data type used as header for custom formats to indicate the exact
// size of the user custom data. This is necessary because it looks
// like GlobalSize() might not return the exact size, but a greater
// value.
typedef uint64_t CustomSizeT;

unsigned long get_shift_from_mask(unsigned long mask) {
  unsigned long shift = 0;
  for (shift=0; shift<sizeof(unsigned long)*8; ++shift)
    if (mask & (1 << shift))
      return shift;
  return shift;
}

class Hglobal {
public:
  Hglobal() : m_handle(nullptr) {
  }

  explicit Hglobal(HGLOBAL handle) : m_handle(handle) {
  }

  explicit Hglobal(size_t len) : m_handle(GlobalAlloc(GHND, len)) {
  }

  ~Hglobal() {
    if (m_handle)
      GlobalFree(m_handle);
  }

  void release() {
    m_handle = nullptr;
  }

  operator HGLOBAL() {
    return m_handle;
  }

private:
  HGLOBAL m_handle;
};

#if CLIP_ENABLE_IMAGE

struct BitmapInfo {
  BITMAPV5HEADER* b5 = nullptr;
  BITMAPINFO* bi = nullptr;
  int width = 0;
  int height = 0;
  uint16_t bit_count = 0;
  uint32_t compression = 0;
  uint32_t red_mask = 0;
  uint32_t green_mask = 0;
  uint32_t blue_mask = 0;
  uint32_t alpha_mask = 0;

  BitmapInfo() {
    // Use DIBV5 only for 32 bpp uncompressed bitmaps and when all
    // masks are valid.
    if (IsClipboardFormatAvailable(CF_DIBV5)) {
      b5 = (BITMAPV5HEADER*)GetClipboardData(CF_DIBV5);
      if (b5 &&
          b5->bV5BitCount == 32 &&
          ((b5->bV5Compression == BI_RGB) ||
           (b5->bV5Compression == BI_BITFIELDS &&
            b5->bV5RedMask && b5->bV5GreenMask &&
            b5->bV5BlueMask && b5->bV5AlphaMask))) {
        width       = b5->bV5Width;
        height      = b5->bV5Height;
        bit_count   = b5->bV5BitCount;
        compression = b5->bV5Compression;
        if (compression == BI_BITFIELDS) {
          red_mask    = b5->bV5RedMask;
          green_mask  = b5->bV5GreenMask;
          blue_mask   = b5->bV5BlueMask;
          alpha_mask  = b5->bV5AlphaMask;
        }
        else {
          red_mask    = 0xff0000;
          green_mask  = 0xff00;
          blue_mask   = 0xff;
          alpha_mask  = 0xff000000;
        }
        return;
      }
    }

    if (IsClipboardFormatAvailable(CF_DIB))
      bi = (BITMAPINFO*)GetClipboardData(CF_DIB);
    if (!bi)
      return;

    width       = bi->bmiHeader.biWidth;
    height      = bi->bmiHeader.biHeight;
    bit_count   = bi->bmiHeader.biBitCount;
    compression = bi->bmiHeader.biCompression;

    if (compression == BI_BITFIELDS) {
      red_mask   = *((uint32_t*)&bi->bmiColors[0]);
      green_mask = *((uint32_t*)&bi->bmiColors[1]);
      blue_mask  = *((uint32_t*)&bi->bmiColors[2]);
      if (bit_count == 32)
        alpha_mask = 0xff000000;
    }
    else if (compression == BI_RGB) {
      switch (bit_count) {
        case 32:
          red_mask   = 0xff0000;
          green_mask = 0xff00;
          blue_mask  = 0xff;
          alpha_mask = 0xff000000;
          break;
        case 24:
        case 8: // We return 8bpp images as 24bpp
          red_mask   = 0xff0000;
          green_mask = 0xff00;
          blue_mask  = 0xff;
          break;
        case 16:
          red_mask   = 0x7c00;
          green_mask = 0x03e0;
          blue_mask  = 0x001f;
          break;
      }
    }
  }

  bool is_valid() const {
    return (b5 || bi);
  }

  void fill_spec(image_spec& spec) {
    spec.width = width;
    spec.height = (height >= 0 ? height: -height);
    // We convert indexed to 24bpp RGB images to match the OS X behavior
    spec.bits_per_pixel = bit_count;
    if (spec.bits_per_pixel <= 8)
      spec.bits_per_pixel = 24;
    spec.bytes_per_row = width*((spec.bits_per_pixel+7)/8);
    spec.red_mask   = red_mask;
    spec.green_mask = green_mask;
    spec.blue_mask  = blue_mask;
    spec.alpha_mask = alpha_mask;

    switch (spec.bits_per_pixel) {

      case 24: {
        // We need one extra byte to avoid a crash updating the last
        // pixel on last row using:
        //
        //   *((uint32_t*)ptr) = pixel24bpp;
        //
        ++spec.bytes_per_row;

        // Align each row to 32bpp
        int padding = (4-(spec.bytes_per_row&3))&3;
        spec.bytes_per_row += padding;
        break;
      }

      case 16: {
        int padding = (4-(spec.bytes_per_row&3))&3;
        spec.bytes_per_row += padding;
        break;
      }
    }

    unsigned long* masks = &spec.red_mask;
    unsigned long* shifts = &spec.red_shift;
    for (unsigned long* shift=shifts, *mask=masks; shift<shifts+4; ++shift, ++mask) {
      if (*mask)
        *shift = get_shift_from_mask(*mask);
    }
  }

};

#endif // CLIP_ENABLE_IMAGE

}

lock::impl::impl(void* hwnd) : m_locked(false) {
  for (int i=0; i<5; ++i) {
    if (OpenClipboard((HWND)hwnd)) {
      m_locked = true;
      break;
    }
    Sleep(20);
  }

  if (!m_locked) {
    error_handler e = get_error_handler();
    if (e)
      e(ErrorCode::CannotLock);
  }
}

lock::impl::~impl() {
  if (m_locked)
    CloseClipboard();
}

bool lock::impl::clear() {
  return (EmptyClipboard() ? true: false);
}

bool lock::impl::is_convertible(format f) const {
  if (f == text_format()) {
    return
      (IsClipboardFormatAvailable(CF_TEXT) ||
       IsClipboardFormatAvailable(CF_UNICODETEXT) ||
       IsClipboardFormatAvailable(CF_OEMTEXT));
  }
#if CLIP_ENABLE_IMAGE
  else if (f == image_format()) {
    return (IsClipboardFormatAvailable(CF_DIB) ? true: false);
  }
#endif // CLIP_ENABLE_IMAGE
  else if (IsClipboardFormatAvailable(f))
    return true;
  else
    return false;
}

bool lock::impl::set_data(format f, const char* buf, size_t len) {
  bool result = false;

  if (f == text_format()) {
    if (len > 0) {
      int reqsize = MultiByteToWideChar(CP_UTF8, 0, buf, len, NULL, 0);
      if (reqsize > 0) {
        ++reqsize;

        Hglobal hglobal(sizeof(WCHAR)*reqsize);
        LPWSTR lpstr = static_cast<LPWSTR>(GlobalLock(hglobal));
        MultiByteToWideChar(CP_UTF8, 0, buf, len, lpstr, reqsize);
        GlobalUnlock(hglobal);

        result = (SetClipboardData(CF_UNICODETEXT, hglobal)) ? true: false;
        if (result)
          hglobal.release();
      }
    }
  }
  else if (f == html_format()) {
    UINT CF_HTML = html_format();

    std::string headerTemplate = 
      "Version:0.9\r\n"
      "StartHTML:00000000\r\n"
      "EndHTML:00000000\r\n"
      "StartFragment:00000000\r\n"
      "EndFragment:00000000\r\n"
      "<html><body>\r\n";

    std::string footerTemplate = 
      "</body>\r\n"
      "</html>";
    std::string fragment = std::string(buf, len);
    std::string fullHtmlContent = headerTemplate + 
      fragment + 
      footerTemplate;

    size_t startHtmlOffset = headerTemplate.find("StartHTML:") + 10;
    size_t endHtmlOffset = headerTemplate.find("EndHTML:") + 8;
    size_t startFragOffset = headerTemplate.find("StartFragment:") + 14;
    size_t endFragOffset = headerTemplate.find("EndFragment:") + 12;


    char offsetBuffer[9];
    sprintf(offsetBuffer, "%08zu", headerTemplate.find("<html>"));
    fullHtmlContent.replace(startHtmlOffset, 8, offsetBuffer);

    sprintf(offsetBuffer, "%08zu", fullHtmlContent.length());
    fullHtmlContent.replace(endHtmlOffset, 8, offsetBuffer);

    sprintf(offsetBuffer, "%08zu", headerTemplate.length());
    fullHtmlContent.replace(startFragOffset, 8, offsetBuffer);

    sprintf(offsetBuffer, "%08zu", headerTemplate.length() + fragment.length());
    fullHtmlContent.replace(endFragOffset, 8, offsetBuffer);

    Hglobal hglobal(fullHtmlContent.size() + 1);
    if (hglobal) {
      auto dst = static_cast<char*>(GlobalLock(hglobal));
      if (dst) {
        memcpy(dst, fullHtmlContent.c_str(), fullHtmlContent.size());
        dst[fullHtmlContent.size()] = '\0';  // Null-terminate
        GlobalUnlock(hglobal);
        
        result = (SetClipboardData(CF_HTML, hglobal) ? true : false);
        if (result)
          hglobal.release();
      }
    }
  }
  else {
    Hglobal hglobal(len+sizeof(CustomSizeT));
    if (hglobal) {
      auto dst = (uint8_t*)GlobalLock(hglobal);
      if (dst) {
        *((CustomSizeT*)dst) = len;
        memcpy(dst+sizeof(CustomSizeT), buf, len);
        GlobalUnlock(hglobal);
        result = (SetClipboardData(f, hglobal) ? true: false);
        if (result)
          hglobal.release();
      }
    }
  }

  return result;
}

bool lock::impl::get_data(format f, char* buf, size_t len) const {
  assert(buf);

  if (!buf || !is_convertible(f))
    return false;

  bool result = false;

  if (f == text_format()) {
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_UNICODETEXT);
      if (hglobal) {
        LPWSTR lpstr = static_cast<LPWSTR>(GlobalLock(hglobal));
        if (lpstr) {
          size_t reqsize =
            WideCharToMultiByte(CP_UTF8, 0, lpstr, -1,
                                nullptr, 0, nullptr, nullptr);

          assert(reqsize <= len);
          if (reqsize <= len) {
            WideCharToMultiByte(CP_UTF8, 0, lpstr, -1,
                                buf, reqsize, nullptr, nullptr);
            result = true;
          }
          GlobalUnlock(hglobal);
        }
      }
    }
    else if (IsClipboardFormatAvailable(CF_TEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_TEXT);
      if (hglobal) {
        LPSTR lpstr = static_cast<LPSTR>(GlobalLock(hglobal));
        if (lpstr) {
          // TODO check length
          memcpy(buf, lpstr, len);
          result = true;
          GlobalUnlock(hglobal);
        }
      }
    }
  }
  else if (f == html_format()) {
    if (IsClipboardFormatAvailable(html_format())) {
      HGLOBAL hglobal = GetClipboardData(html_format());
      if (hglobal) {
        LPSTR lpstr = (LPSTR)GlobalLock(hglobal);
        if (lpstr) {
          // TODO check length
          std::string html = std::string(lpstr); 
          std::smatch sMatches, eMatches;
          regex_search(html, sMatches, std::regex("StartFragment:(\\d+)"));
          regex_search(html, eMatches, std::regex("EndFragment:(\\d+)"));
          if(sMatches.size() > 1 && eMatches.size() > 1) {
            size_t fragmentStart = std::stoi(sMatches[1]);
            size_t fragmentEnd = std::stoi(eMatches[1]);
            std::string fragment = html.substr(fragmentStart, fragmentEnd - fragmentStart);
            memcpy(buf, fragment.c_str(), len);
            result = true;
          }
          GlobalUnlock(hglobal);
        }
      }
    }
  }
  else {
    if (IsClipboardFormatAvailable(f)) {
      HGLOBAL hglobal = GetClipboardData(f);
      if (hglobal) {
        const SIZE_T total_size = GlobalSize(hglobal);
        auto ptr = (const uint8_t*)GlobalLock(hglobal);
        if (ptr) {
          CustomSizeT reqsize = *((CustomSizeT*)ptr);

          // If the registered length of data in the first CustomSizeT
          // number of bytes of the hglobal data is greater than the
          // GlobalSize(hglobal), something is wrong, it should not
          // happen.
          assert(reqsize <= total_size);
          if (reqsize > total_size)
            reqsize = total_size - sizeof(CustomSizeT);

          if (reqsize <= len) {
            memcpy(buf, ptr+sizeof(CustomSizeT), reqsize);
            result = true;
          }
          GlobalUnlock(hglobal);
        }
      }
    }
  }

  return result;
}

size_t lock::impl::get_data_length(format f) const {
  size_t len = 0;

  if (f == text_format()) {
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_UNICODETEXT);
      if (hglobal) {
        LPWSTR lpstr = static_cast<LPWSTR>(GlobalLock(hglobal));
        if (lpstr) {
          len =
            WideCharToMultiByte(CP_UTF8, 0, lpstr, -1,
                                nullptr, 0, nullptr, nullptr);
          GlobalUnlock(hglobal);
        }
      }
    }
    else if (IsClipboardFormatAvailable(CF_TEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_TEXT);
      if (hglobal) {
        LPSTR lpstr = (LPSTR)GlobalLock(hglobal);
        if (lpstr) {
          len = strlen(lpstr) + 1;
          GlobalUnlock(hglobal);
        }
      }
    }
  }
  else if(f == html_format()) {
    if (IsClipboardFormatAvailable(html_format())) {
      HGLOBAL hglobal = GetClipboardData(html_format());
      if (hglobal) {
        LPSTR lpstr = (LPSTR)GlobalLock(hglobal);
        if (lpstr) {
          len = strlen(lpstr) + 1;
          GlobalUnlock(hglobal);
        }
      }
    }
  }
  else if (f != empty_format()) {
    if (IsClipboardFormatAvailable(f)) {
      HGLOBAL hglobal = GetClipboardData(f);
      if (hglobal) {
        const SIZE_T total_size = GlobalSize(hglobal);
        auto ptr = (const uint8_t*)GlobalLock(hglobal);
        if (ptr) {
          len = *((CustomSizeT*)ptr);

          assert(len <= total_size);
          if (len > total_size)
            len = total_size - sizeof(CustomSizeT);

          GlobalUnlock(hglobal);
        }
      }
    }
  }

  return len;
}

#if CLIP_ENABLE_IMAGE

bool lock::impl::set_image(const image& image) {
  const image_spec& spec = image.spec();

  // Add the PNG clipboard format for images with alpha channel
  // (useful to communicate with some Windows programs that only use
  // alpha data from PNG clipboard format)
  if (spec.bits_per_pixel == 32 &&
      spec.alpha_mask) {
    UINT png_format = RegisterClipboardFormatA("PNG");
    if (png_format) {
      Hglobal png_handle(win::write_png(image));
      if (png_handle)
        SetClipboardData(png_format, png_handle);
    }
  }

  image_spec out_spec = spec;

  int palette_colors = 0;
  int padding = 0;
  switch (spec.bits_per_pixel) {
    case 24: padding = (4-((spec.width*3)&3))&3; break;
    case 16: padding = ((4-((spec.width*2)&3))&3)/2; break;
    case 8:  padding = (4-(spec.width&3))&3; break;
  }
  out_spec.bytes_per_row += padding;

  // Create the BITMAPV5HEADER structure
  Hglobal hmem(
    GlobalAlloc(
      GHND,
      sizeof(BITMAPV5HEADER)
      + palette_colors*sizeof(RGBQUAD)
      + out_spec.bytes_per_row*out_spec.height));
  if (!hmem)
    return false;

  out_spec.red_mask    = 0x00ff0000;
  out_spec.green_mask  = 0xff00;
  out_spec.blue_mask   = 0xff;
  out_spec.alpha_mask  = 0xff000000;
  out_spec.red_shift   = 16;
  out_spec.green_shift = 8;
  out_spec.blue_shift  = 0;
  out_spec.alpha_shift = 24;

  BITMAPV5HEADER* bi = (BITMAPV5HEADER*)GlobalLock(hmem);
  bi->bV5Size = sizeof(BITMAPV5HEADER);
  bi->bV5Width = out_spec.width;
  bi->bV5Height = out_spec.height;
  bi->bV5Planes = 1;
  bi->bV5BitCount = (WORD)out_spec.bits_per_pixel;
  bi->bV5Compression = BI_RGB;
  bi->bV5SizeImage = out_spec.bytes_per_row*spec.height;
  bi->bV5RedMask   = out_spec.red_mask;
  bi->bV5GreenMask = out_spec.green_mask;
  bi->bV5BlueMask  = out_spec.blue_mask;
  bi->bV5AlphaMask = out_spec.alpha_mask;
  bi->bV5CSType = LCS_WINDOWS_COLOR_SPACE;
  bi->bV5Intent = LCS_GM_GRAPHICS;
  bi->bV5ClrUsed = 0;

  switch (spec.bits_per_pixel) {
    case 32: {
      const char* src = image.data();
      char* dst = (((char*)bi)+bi->bV5Size) + (out_spec.height-1)*out_spec.bytes_per_row;
      for (long y=spec.height-1; y>=0; --y) {
        const uint32_t* src_x = (const uint32_t*)src;
        uint32_t* dst_x = (uint32_t*)dst;

        for (unsigned long x=0; x<spec.width; ++x, ++src_x, ++dst_x) {
          uint32_t c = *src_x;
          int r = ((c & spec.red_mask  ) >> spec.red_shift  );
          int g = ((c & spec.green_mask) >> spec.green_shift);
          int b = ((c & spec.blue_mask ) >> spec.blue_shift );
          int a = ((c & spec.alpha_mask) >> spec.alpha_shift);

          // Windows requires premultiplied RGBA values
          r = r * a / 255;
          g = g * a / 255;
          b = b * a / 255;

          *dst_x =
            (r << out_spec.red_shift  ) |
            (g << out_spec.green_shift) |
            (b << out_spec.blue_shift ) |
            (a << out_spec.alpha_shift);
        }

        src += spec.bytes_per_row;
        dst -= out_spec.bytes_per_row;
      }
      break;
    }
    default:
      error_handler e = get_error_handler();
      if (e)
        e(ErrorCode::ImageNotSupported);
      return false;
  }

  GlobalUnlock(hmem);
  SetClipboardData(CF_DIBV5, hmem);
  return true;
}

bool lock::impl::get_image(image& output_img) const {
  // Get the "PNG" clipboard format (this is useful only for 32bpp
  // images with alpha channel, in other case we can use the regular
  // DIB format)
  UINT png_format = RegisterClipboardFormatA("PNG");
  if (png_format && IsClipboardFormatAvailable(png_format)) {
    HANDLE png_handle = GetClipboardData(png_format);
    if (png_handle) {
      size_t png_size = GlobalSize(png_handle);
      uint8_t* png_data = (uint8_t*)GlobalLock(png_handle);
      bool result = win::read_png(png_data, png_size, &output_img, nullptr);
      GlobalUnlock(png_handle);
      if (result)
        return true;
    }
  }

  BitmapInfo bi;
  if (!bi.is_valid()) {
    // There is no image at all in the clipboard, no need to report
    // this as an error, just return false.
    return false;
  }

  image_spec spec;
  bi.fill_spec(spec);
  image img(spec);

  switch (bi.bit_count) {

    case 32:
    case 24:
    case 16: {
      const uint8_t* src = nullptr;

      if (bi.compression == BI_RGB ||
          bi.compression == BI_BITFIELDS) {
        if (bi.b5)
          src = ((uint8_t*)bi.b5) + bi.b5->bV5Size;
        else
          src = ((uint8_t*)bi.bi) + bi.bi->bmiHeader.biSize;
        if (bi.compression == BI_BITFIELDS)
          src += sizeof(RGBQUAD)*3;
      }

      if (src) {
        const int src_bytes_per_row = spec.width*((bi.bit_count+7)/8);
        const int padding = (4-(src_bytes_per_row&3))&3;

        for (long y=spec.height-1; y>=0; --y, src+=src_bytes_per_row+padding) {
          char* dst = img.data()+y*spec.bytes_per_row;
          std::copy(src, src+src_bytes_per_row, dst);
        }
      }

      // Windows uses premultiplied RGB values, and we use straight
      // alpha. So we have to divide all RGB values by its alpha.
      if (bi.bit_count == 32 && spec.alpha_mask) {
        details::divide_rgb_by_alpha(img);
      }
      break;
    }

    case 8: {
      assert(bi.bi);

      const int colors = (bi.bi->bmiHeader.biClrUsed > 0 ? bi.bi->bmiHeader.biClrUsed: 256);
      std::vector<uint32_t> palette(colors);
      for (int c=0; c<colors; ++c) {
        palette[c] =
          (bi.bi->bmiColors[c].rgbRed   << spec.red_shift) |
          (bi.bi->bmiColors[c].rgbGreen << spec.green_shift) |
          (bi.bi->bmiColors[c].rgbBlue  << spec.blue_shift);
      }

      const uint8_t* src = (((uint8_t*)bi.bi) + bi.bi->bmiHeader.biSize + sizeof(RGBQUAD)*colors);
      const int padding = (4-(spec.width&3))&3;

      for (long y=spec.height-1; y>=0; --y, src+=padding) {
        char* dst = img.data()+y*spec.bytes_per_row;

        for (unsigned long x=0; x<spec.width; ++x, ++src, dst+=3) {
          int idx = *src;
          if (idx < 0)
            idx = 0;
          else if (idx >= colors)
            idx = colors-1;

          *((uint32_t*)dst) = palette[idx];
        }
      }
      break;
    }
  }

  std::swap(output_img, img);
  return true;
}

bool lock::impl::get_image_spec(image_spec& spec) const {
  UINT png_format = RegisterClipboardFormatA("PNG");
  if (png_format && IsClipboardFormatAvailable(png_format)) {
    HANDLE png_handle = GetClipboardData(png_format);
    if (png_handle) {
      size_t png_size = GlobalSize(png_handle);
      uint8_t* png_data = (uint8_t*)GlobalLock(png_handle);
      bool result = win::read_png(png_data, png_size, nullptr, &spec);
      GlobalUnlock(png_handle);
      if (result)
        return true;
    }
  }

  BitmapInfo bi;
  if (!bi.is_valid())
    return false;
  bi.fill_spec(spec);
  return true;
}

#endif // CLIP_ENABLE_IMAGE

format register_format(const std::string& name) {
  int reqsize = 1+MultiByteToWideChar(CP_UTF8, 0,
                                      name.c_str(), name.size(), NULL, 0);
  std::vector<WCHAR> buf(reqsize);
  MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(),
                      &buf[0], reqsize);

  // From MSDN, registered clipboard formats are identified by values
  // in the range 0xC000 through 0xFFFF.
  return (format)RegisterClipboardFormatW(&buf[0]);
}

} // namespace clip
