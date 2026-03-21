#ifndef DCEARTH_MATERIAL_HPP
#define DCEARTH_MATERIAL_HPP

#include "dcearth/vertex.hpp"
#include <cmath>
#include <dc/pvr.h>

namespace dcearth {

class material {
  public:
    virtual ~material() = default;
    virtual void bind() const = 0;
    virtual void shade(pvr_vertex_t &pv, const vertex &v) const = 0;
};

class color_material : public material {
  public:
    vec3 base_color = {1.0f, 1.0f, 1.0f};
    vec3 light_dir = {0.0f, 1.0f, 0.0f};
    vec3 light_color = {1.0f, 1.0f, 1.0f};
    float ambient = 0.2f;
    float diffuse = 0.8f;
    float min_intensity = 0.1f;
    int list = PVR_LIST_OP_POLY;

    void bind() const override {
        if (!compiled_)
            compile();
        pvr_list_begin(list);
        pvr_prim(&header_, sizeof(header_));
    }

    void shade(pvr_vertex_t &pv, const vertex &v) const override {
        float ndotl = dot(v.normal, light_dir);
        float i = fmaxf(min_intensity, fminf(1.0f, ndotl * diffuse + ambient));
        auto r = uint32_t(base_color[0] * light_color[0] * 255.0f * i);
        auto g = uint32_t(base_color[1] * light_color[1] * 255.0f * i);
        auto b = uint32_t(base_color[2] * light_color[2] * 255.0f * i);
        pv.argb = 0xff000000 | (r << 16) | (g << 8) | b;
    }

  private:
    void compile() const {
        pvr_poly_cxt_t cxt;
        pvr_poly_cxt_col(&cxt, list);
        cxt.gen.shading = PVR_SHADE_GOURAUD;
        pvr_poly_compile(&header_, &cxt);
        compiled_ = true;
    }

    mutable pvr_poly_hdr_t header_;
    mutable bool compiled_ = false;
};

} // namespace dcearth

#endif
