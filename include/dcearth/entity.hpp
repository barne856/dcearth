#ifndef DCEARTH_ENTITY_HPP
#define DCEARTH_ENTITY_HPP

#include <array>
#include <cstdint>

namespace dcearth {

struct keyboard_input_state {
  std::array<uint8_t, 256> keys = {};
  uint8_t modifiers = 0;

  bool is_down(uint8_t key) const { return (keys[key] & 0x1u) != 0; }
  bool changed_down(uint8_t key) const { return keys[key] == 0x1u; }
  bool changed_up(uint8_t key) const { return keys[key] == 0x2u; }
};

struct mouse_input_state {
  uint32_t buttons = 0;
  int dx = 0;
  int dy = 0;
  int dz = 0;
};

class entity {
public:
  virtual ~entity() = default;
  virtual void on_enter() {}
  virtual void on_exit() {}
  virtual void on_render(float dt) {}
  virtual void on_update(float dt) {}
  virtual void on_button(uint32_t buttons) {}
  virtual void on_joystick(float jx, float jy, float j2x, float j2y) {}
  virtual void on_trigger(float lt, float rt) {}
  virtual void on_keyboard(const keyboard_input_state &keyboard) {}
  virtual void on_mouse(const mouse_input_state &mouse) {}
};

} // namespace dcearth

#endif
