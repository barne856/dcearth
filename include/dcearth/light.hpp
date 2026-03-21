#ifndef DCEARTH_LIGHT_HPP
#define DCEARTH_LIGHT_HPP

#include <squint/squint.hpp>

namespace dcearth {

using namespace squint;

struct directional_light {
    vec3 direction;
    vec3 color = {1.0f, 1.0f, 1.0f};
};

} // namespace dcearth

#endif
