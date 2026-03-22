#ifndef DCEARTH_MESH_HPP
#define DCEARTH_MESH_HPP

#include "dcearth/material.hpp"
#include "dcearth/renderer.hpp"
#include <vector>

namespace dcearth {

class mesh {
public:
  void draw(const material *mat, const mat4 &mvp) const {
    float half_w = renderer::get().screen_width() * 0.5f;
    float half_h = renderer::get().screen_height() * 0.5f;
    pvr_vertex_t pv{};
    for (auto &s : strips_) {
      for (size_t i = 0; i < s.size(); i++) {
        auto &v = s[i];
        vec4 clip =
            mvp * vec4{v.position[0], v.position[1], v.position[2], 1.0f};
        float w = clip[3];
        pv.x = (clip[0] / w + 1.0f) * half_w;
        pv.y = (1.0f - clip[1] / w) * half_h;
        pv.z = 1.0f / w;
        mat->shade(pv, v);
        pv.flags = (i == s.size() - 1) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
        pvr_prim(&pv, sizeof(pv));
      }
    }
  }

protected:
  std::vector<std::vector<vertex>> strips_;
};

} // namespace dcearth

#endif
