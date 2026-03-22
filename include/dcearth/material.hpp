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

} // namespace dcearth

#endif
