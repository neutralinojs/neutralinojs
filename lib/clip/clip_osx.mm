// Clip Library
// Copyright (c) 2015-2023 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip.h"
#include "clip_common.h"
#include "clip_lock_impl.h"

#include <cassert>
#include <vector>
#include <map>

#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

namespace clip {

namespace {

  format g_last_format = 100;
  std::map<std::string, format> g_name_to_format;
  std::map<format, std::string> g_format_to_name;

#if CLIP_ENABLE_IMAGE

  bool get_image_from_clipboard(image* output_img,
                                image_spec* output_spec)
  {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    NSString* result = [pasteboard availableTypeFromArray:
                                [NSArray arrayWithObjects:NSPasteboardTypeTIFF,NSPasteboardTypePNG,nil]];

    if (!result)
      return false;

    NSData* data = [pasteboard dataForType:result];
    if (!data)
      return false;

    NSBitmapImageRep* bitmap = [NSBitmapImageRep imageRepWithData:data];

    if ((bitmap.bitmapFormat & NSBitmapFormatFloatingPointSamples) ||
        (bitmap.planar)) {
      error_handler e = get_error_handler();
      if (e)
        e(ErrorCode::ImageNotSupported);
      return false;
    }

    image_spec spec;
    spec.width = bitmap.pixelsWide;
    spec.height = bitmap.pixelsHigh;
    spec.bits_per_pixel = bitmap.bitsPerPixel;
    spec.bytes_per_row = bitmap.bytesPerRow;

    // We need three samples for Red/Green/Blue
    if (bitmap.samplesPerPixel >= 3) {
      // Here we are guessing the bits per sample (generally 8, not
      // sure how many bits per sample macOS uses for 16bpp
      // NSBitmapFormat or if this format is even used).
      int bits_per_sample = (bitmap.bitsPerPixel == 16 ? 5: 8);
      int bits_shift = 0;

      // With alpha
      if (bitmap.alpha) {
        if (bitmap.bitmapFormat & NSBitmapFormatAlphaFirst) {
          spec.alpha_shift = 0;
          bits_shift += bits_per_sample;
        }
        else {
          spec.alpha_shift = 3*bits_per_sample;
        }
      }

      unsigned long* masks = &spec.red_mask;
      unsigned long* shifts = &spec.red_shift;

      // Red/green/blue shifts
      for (unsigned long* shift=shifts; shift<shifts+3; ++shift) {
        *shift = bits_shift;
        bits_shift += bits_per_sample;
      }

      // With alpha
      if (bitmap.alpha) {
        if (bitmap.bitmapFormat & NSBitmapFormatSixteenBitBigEndian ||
            bitmap.bitmapFormat & NSBitmapFormatThirtyTwoBitBigEndian) {
          std::swap(spec.red_shift, spec.alpha_shift);
          std::swap(spec.green_shift, spec.blue_shift);
        }
      }
      // Without alpha
      else {
        if (bitmap.bitmapFormat & NSBitmapFormatSixteenBitBigEndian ||
            bitmap.bitmapFormat & NSBitmapFormatThirtyTwoBitBigEndian) {
          std::swap(spec.red_shift, spec.blue_shift);
        }
      }

      // Calculate all masks
      for (unsigned long* shift=shifts, *mask=masks; shift<shifts+4; ++shift, ++mask)
        *mask = ((1ul<<bits_per_sample)-1ul) << (*shift);

      // Without alpha
      if (!bitmap.alpha)
        spec.alpha_mask = 0;
    }

    if (output_spec) {
      *output_spec = spec;
    }

    if (output_img) {
      unsigned long size = spec.bytes_per_row*spec.height;
      image img(spec);

      std::copy(bitmap.bitmapData,
                bitmap.bitmapData+size, img.data());

      // Convert premultiplied data to unpremultiplied if needed.
      if (bitmap.alpha &&
          bitmap.samplesPerPixel >= 3 &&
          !(bitmap.bitmapFormat & NSBitmapFormatAlphaNonpremultiplied)) {
        details::divide_rgb_by_alpha(
          img,
          true); // hasAlphaGreaterThanZero=true because we have valid alpha information
      }

      std::swap(*output_img, img);
    }

    return true;
  }

#endif // CLIP_ENABLE_IMAGE

}

lock::impl::impl(void*) : m_locked(true) {
}

lock::impl::~impl() {
}

bool lock::impl::clear() {
  @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    return true;
  }
}

bool lock::impl::is_convertible(format f) const {
  @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    NSString* result = nil;

    if (f == text_format()) {
      result = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:NSPasteboardTypeString]];
    }
#if CLIP_ENABLE_IMAGE
    else if (f == image_format()) {
      result = [pasteboard availableTypeFromArray:
                        [NSArray arrayWithObjects:NSPasteboardTypeTIFF,NSPasteboardTypePNG,nil]];
    }
#endif // CLIP_ENABLE_IMAGE
    else {
      auto it = g_format_to_name.find(f);
      if (it != g_format_to_name.end()) {
        const std::string& name = it->second;
        NSString* string = [[NSString alloc] initWithBytesNoCopy:(void*)name.c_str()
                                                          length:name.size()
                                                        encoding:NSUTF8StringEncoding
                                                    freeWhenDone:NO];
        result = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:string]];
      }
    }

    return (result ? true: false);
  }
}

bool lock::impl::set_data(format f, const char* buf, size_t len) {
  @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (f == text_format()) {
      NSString* string = [[NSString alloc] initWithBytesNoCopy:(void*)buf
                                                        length:len
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
      [pasteboard setString:string forType:NSPasteboardTypeString];
      return true;
    }
    else {
      auto it = g_format_to_name.find(f);
      if (it != g_format_to_name.end()) {
        const std::string& formatName = it->second;
        NSString* typeString = [[NSString alloc]
                                 initWithBytesNoCopy:(void*)formatName.c_str()
                                              length:formatName.size()
                                            encoding:NSUTF8StringEncoding
                                        freeWhenDone:NO];
        NSData* data = [NSData dataWithBytesNoCopy:(void*)buf
                                            length:len
                                      freeWhenDone:NO];

        if ([pasteboard setData:data forType:typeString])
          return true;
      }
    }
    return false;
  }
}

bool lock::impl::get_data(format f, char* buf, size_t len) const {
  @autoreleasepool {
    assert(buf);
    if (!buf || !is_convertible(f))
      return false;

    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (f == text_format()) {
      NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
      int reqsize = [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding]+1;

      assert(reqsize <= len);
      if (reqsize > len) {
        // Buffer is too small
        return false;
      }

      if (reqsize == 0)
        return true;

      memcpy(buf, [string UTF8String], reqsize);
      return true;
    }

    auto it = g_format_to_name.find(f);
    if (it == g_format_to_name.end())
      return false;

    const std::string& formatName = it->second;
    NSString* typeString =
      [[NSString alloc] initWithBytesNoCopy:(void*)formatName.c_str()
                                     length:formatName.size()
                                   encoding:NSUTF8StringEncoding
                               freeWhenDone:NO];

    NSData* data = [pasteboard dataForType:typeString];
    if (!data)
      return false;

    [data getBytes:buf length:len];
    return true;
  }
}

size_t lock::impl::get_data_length(format f) const {
  @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (f == text_format()) {
      NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
      return [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding]+1;
    }

    auto it = g_format_to_name.find(f);
    if (it == g_format_to_name.end())
      return 0;

    const std::string& formatName = it->second;
    NSString* typeString =
      [[NSString alloc] initWithBytesNoCopy:(void*)formatName.c_str()
                                     length:formatName.size()
                                   encoding:NSUTF8StringEncoding
                               freeWhenDone:NO];

    NSData* data = [pasteboard dataForType:typeString];
    if (!data)
      return 0;

    return data.length;
  }
}

#if CLIP_ENABLE_IMAGE

bool lock::impl::set_image(const image& image) {
  @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    const image_spec& spec = image.spec();

    NSBitmapFormat bitmapFormat = 0;
    int samples_per_pixel = 0;
    if (spec.alpha_mask) {
      samples_per_pixel = 4;
      if (spec.alpha_shift == 0)
        bitmapFormat |= NSBitmapFormatAlphaFirst;
      bitmapFormat |= NSBitmapFormatAlphaNonpremultiplied;
    }
    else if (spec.red_mask || spec.green_mask || spec.blue_mask) {
      samples_per_pixel = 3;
    }
    else {
      samples_per_pixel = 1;
    }

    if (spec.bits_per_pixel == 32)
      bitmapFormat |= NSBitmapFormatThirtyTwoBitLittleEndian;
    else if (spec.bits_per_pixel == 16)
      bitmapFormat |= NSBitmapFormatSixteenBitLittleEndian;

    std::vector<unsigned char*> planes(1);
    planes[0] = (unsigned char*)image.data();

    NSBitmapImageRep* bitmap =
      [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:&planes[0]
                      pixelsWide:spec.width
                      pixelsHigh:spec.height
                   bitsPerSample:spec.bits_per_pixel / samples_per_pixel
                 samplesPerPixel:samples_per_pixel
                        hasAlpha:(spec.alpha_mask ? YES: NO)
                        isPlanar:NO
                  colorSpaceName:NSDeviceRGBColorSpace
                    bitmapFormat:bitmapFormat
                     bytesPerRow:spec.bytes_per_row
                    bitsPerPixel:spec.bits_per_pixel];
    if (!bitmap)
      return false;

    NSData* data = bitmap.TIFFRepresentation;
    if (!data)
      return false;

    if ([pasteboard setData:data forType:NSPasteboardTypeTIFF])
      return true;

    return false;
  }
}

bool lock::impl::get_image(image& img) const {
  return get_image_from_clipboard(&img, nullptr);
}

bool lock::impl::get_image_spec(image_spec& spec) const {
  return get_image_from_clipboard(nullptr, &spec);
}

#endif // CLIP_ENABLE_IMAGE

format register_format(const std::string& name) {
  // Check if the format is already registered
  auto it = g_name_to_format.find(name);
  if (it != g_name_to_format.end())
    return it->second;

  format new_format = g_last_format++;
  g_name_to_format[name] = new_format;
  g_format_to_name[new_format] = name;
  return new_format;
}

} // namespace clip
