#ifndef DCEARTH_TILE_SOURCE_HPP
#define DCEARTH_TILE_SOURCE_HPP

#include "dcearth/tile_id.hpp"
#include <cinttypes>
#include <cstdint>
#include <cstdio>

extern "C" {
#include <jpeg/jpeglib.h>
}

namespace dcearth {

inline bool decode_jpeg_to_rgb565(const char *path, uint16_t *out, int size) {
  FILE *f = fopen(path, "r");
  if (!f)
    return false;

  jpeg_decompress_struct cinfo;
  jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, f);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  uint8_t row_buf[256 * 3];
  JSAMPROW row_ptr = row_buf;

  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, &row_ptr, 1);
    uint16_t *dst = out + size * (cinfo.output_scanline - 1);
    for (int i = 0; i < size; i++)
      dst[i] = ((row_buf[i * 3] >> 3) << 11) |
               ((row_buf[i * 3 + 1] >> 2) << 5) | (row_buf[i * 3 + 2] >> 3);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(f);
  return true;
}

class tile_source {
public:
  virtual ~tile_source() = default;
  virtual bool fetch(tile_id id, int month, uint16_t *rgb565_out, int size) = 0;
};

class file_tile_source : public tile_source {
public:
  file_tile_source(const char *base_path) : base_path_(base_path) {}

  bool fetch(tile_id id, int month, uint16_t *rgb565_out, int size) override {
    char path[256];
    snprintf(path, sizeof(path), "%s/%d/%d/%" PRIu32 "_%" PRIu32 ".jpg",
             base_path_, month, id.lod, id.x, id.y);
    return decode_jpeg_to_rgb565(path, rgb565_out, size);
  }

private:
  const char *base_path_;
};

} // namespace dcearth

#endif
