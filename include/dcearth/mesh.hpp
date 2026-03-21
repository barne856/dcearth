#ifndef DCEARTH_MESH_HPP
#define DCEARTH_MESH_HPP

#include "dcearth/material.hpp"
#include <cmath>
#include <vector>

namespace dcearth {

class mesh {
  public:
    void draw(const material *mat, const mat4 &mvp) const {
        mat->bind();
        pvr_vertex_t pv{};
        for (auto &s : strips_) {
            for (size_t i = 0; i < s.size(); i++) {
                auto &v = s[i];
                vec4 clip = mvp * vec4{v.position[0], v.position[1], v.position[2], 1.0f};
                float w = clip[3];
                pv.x = (clip[0] / w + 1.0f) * 320.0f;
                pv.y = (1.0f - clip[1] / w) * 240.0f;
                pv.z = 1.0f / w;
                mat->shade(pv, v);
                pv.flags = (i == s.size() - 1) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
                pvr_prim((void *)&pv, sizeof(pv));
            }
        }
    }

  protected:
    std::vector<std::vector<vertex>> strips_;
};

class sphere_mesh : public mesh {
  public:
    sphere_mesh(float radius, int slices, int stacks) {
        for (int i = 0; i < stacks; i++) {
            float theta0 = float(M_PI) * i / stacks;
            float theta1 = float(M_PI) * (i + 1) / stacks;
            std::vector<vertex> s;
            s.reserve((slices + 1) * 2);
            for (int j = 0; j <= slices; j++) {
                float phi = 2.0f * float(M_PI) * j / slices;
                float cp = cosf(phi), sp = sinf(phi);
                float st0 = sinf(theta0), ct0 = cosf(theta0);
                float st1 = sinf(theta1), ct1 = cosf(theta1);
                vec3 n0 = {st0 * cp, ct0, st0 * sp};
                vec3 n1 = {st1 * cp, ct1, st1 * sp};
                s.push_back({n0 * radius, n0});
                s.push_back({n1 * radius, n1});
            }
            strips_.push_back(std::move(s));
        }
    }
};

} // namespace dcearth

#endif
