#ifndef DCEARTH_TILE_MATERIAL_HPP
#define DCEARTH_TILE_MATERIAL_HPP

#include "dcearth/material.hpp"

namespace dcearth {

class tile_material : public material {
public:
  vec3 light_dir = {0.0f, 1.0f, 0.0f};
  float ambient = 0.15f;
  float diffuse = 0.85f;
  float min_intensity = 0.08f;

  void set_header(pvr_poly_hdr_t *hdr) { current_header_ = hdr; }

  void bind() const override {
    pvr_prim(current_header_, sizeof(pvr_poly_hdr_t));
  }

  void shade(pvr_vertex_t &pv, const vertex &v) const override {
    float ndotl = dot(v.normal, light_dir);
    float i = fmaxf(min_intensity, fminf(1.0f, ndotl * diffuse + ambient));
    auto c = uint32_t(i * 255.0f);
    pv.argb = 0xff000000 | (c << 16) | (c << 8) | c;
    pv.u = v.uv[0];
    pv.v = v.uv[1];
  }

private:
  pvr_poly_hdr_t *current_header_ = nullptr;
};

} // namespace dcearth

#endif
