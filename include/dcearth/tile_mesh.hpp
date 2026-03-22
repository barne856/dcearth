#ifndef DCEARTH_TILE_MESH_HPP
#define DCEARTH_TILE_MESH_HPP

#include "dcearth/mesh.hpp"
#include "dcearth/tile_id.hpp"
#include <cmath>

namespace dcearth {

class tile_mesh : public mesh {
public:
  static constexpr int MAX_GRID = 9;
  static constexpr float HALF_TEXEL = 0.5f / 256.0f;

  static int subdivs_for_lod(int lod) {
    if (lod <= 2)
      return 8;
    if (lod <= 3)
      return 4;
    return 2;
  }

  void set_tile(tile_id t) {
    if (t == cached_tile_)
      return;
    cached_tile_ = t;

    subdivs_ = subdivs_for_lod(t.lod);
    int grid = subdivs_ + 1;
    auto b = bounds_of(t);

    double cl[MAX_GRID], sl[MAX_GRID], cp[MAX_GRID], sp[MAX_GRID];
    for (int i = 0; i < grid; i++) {
      double frac = double(i) / double(subdivs_);
      double lon = b.lon_min + (b.lon_max - b.lon_min) * frac;
      double lat = b.lat_max - (b.lat_max - b.lat_min) * frac;
      cp[i] = cos(lon);
      sp[i] = sin(lon);
      cl[i] = cos(lat);
      sl[i] = sin(lat);
    }

    for (int lat_i = 0; lat_i < grid; lat_i++)
      for (int lon_i = 0; lon_i < grid; lon_i++)
        world_[lat_i][lon_i] = {cl[lat_i] * cp[lon_i], sl[lat_i],
                                -cl[lat_i] * sp[lon_i]};
  }

  void generate(tile_id t, float u_off, float v_off, float u_scale,
                float v_scale, const dvec3 &cam_pos) {
    set_tile(t);
    int grid = subdivs_ + 1;
    float inv_subdivs = 1.0f / float(subdivs_);

    float u0 = u_off + HALF_TEXEL;
    float u1 = u_off + u_scale - HALF_TEXEL;
    float v0 = v_off + HALF_TEXEL;
    float v1 = v_off + v_scale - HALF_TEXEL;

    strips_.resize(subdivs_);
    for (int row = 0; row < subdivs_; row++) {
      auto &strip = strips_[row];
      strip.clear();
      strip.reserve(grid * 2);

      for (int col = 0; col < grid; col++) {
        float s_frac = float(col) * inv_subdivs;

        for (int r = 1; r >= 0;
             r--) { // reversed for correct winding after Z negate
          int lat_idx = row + r;
          dvec3 &wp = world_[lat_idx][col];

          // subtract at double precision before casting to float to avoid
          // jitter at high zoom
          dvec3 rel = wp - cam_pos;
          vec3 pos = {float(rel[0]), float(rel[1]), float(rel[2])};
          vec3 normal = {float(wp[0]), float(wp[1]), float(wp[2])};

          float t_frac = float(lat_idx) * inv_subdivs;
          float u = u0 + s_frac * (u1 - u0);
          float v = v0 + t_frac * (v1 - v0);

          strip.push_back({pos, normal, {u, v}});
        }
      }
    }
  }

private:
  tile_id cached_tile_ = {255, 0, 0};
  int subdivs_ = 0;
  dvec3 world_[MAX_GRID][MAX_GRID];
};

} // namespace dcearth

#endif
