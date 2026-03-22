#ifndef DCEARTH_TILE_ID_HPP
#define DCEARTH_TILE_ID_HPP

#include <array>
#include <cmath>
#include <cstdint>
#include <squint/squint.hpp>

namespace dcearth {

using namespace squint;

struct tile_id {
  uint8_t lod;
  uint32_t x, y;

  bool operator==(const tile_id &o) const {
    return lod == o.lod && x == o.x && y == o.y;
  }
};

struct tile_bounds {
  double lon_min, lon_max, lat_min, lat_max;
};

inline uint64_t tile_key(tile_id t) {
  return (uint64_t(t.lod) << 48) | (uint64_t(t.x) << 24) | uint64_t(t.y);
}

inline uint32_t tile_cols(uint8_t lod) { return 1u << (lod + 1); }
inline uint32_t tile_rows(uint8_t lod) { return 1u << lod; }

inline tile_bounds bounds_of(tile_id t) {
  double cols = double(tile_cols(t.lod));
  double rows = double(tile_rows(t.lod));
  double lon_min = (t.x / cols) * 2.0 * M_PI - M_PI;
  double lon_max = ((t.x + 1) / cols) * 2.0 * M_PI - M_PI;
  double lat_max = M_PI / 2.0 - (t.y / rows) * M_PI;
  double lat_min = M_PI / 2.0 - ((t.y + 1) / rows) * M_PI;
  return {lon_min, lon_max, lat_min, lat_max};
}

inline tile_id parent(tile_id t) {
  return {uint8_t(t.lod - 1), t.x / 2, t.y / 2};
}

inline std::array<tile_id, 4> children(tile_id t) {
  uint8_t cl = uint8_t(t.lod + 1);
  uint32_t cx = t.x * 2, cy = t.y * 2;
  return {
      {{cl, cx, cy}, {cl, cx + 1, cy}, {cl, cx, cy + 1}, {cl, cx + 1, cy + 1}}};
}

inline dvec3 center_on_sphere(tile_id t) {
  auto b = bounds_of(t);
  double lon = (b.lon_min + b.lon_max) * 0.5;
  double lat = (b.lat_min + b.lat_max) * 0.5;
  double cl = cos(lat), sl = sin(lat);
  double cp = cos(lon), sp = sin(lon);
  // Z negated so positive longitude goes right when viewed from outside
  return dvec3{cl * cp, sl, -cl * sp};
}

} // namespace dcearth

#endif
