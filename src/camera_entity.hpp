#ifndef DCEARTH_CAMERA_ENTITY_HPP
#define DCEARTH_CAMERA_ENTITY_HPP

#include "dcearth/entity.hpp"
#include <cmath>
#include <dc/maple/keyboard.h>
#include <dc/maple/mouse.h>
#include <squint/squint.hpp>

namespace dcearth {

using namespace squint;
using namespace squint::geometry;

constexpr float mouse_orbit_sensitivity = 0.08f;
constexpr double mouse_zoom_sensitivity = 0.08;

class orbital_camera : public entity {
public:
  double latitude = 0.0;
  double longitude = 0.0;
  double altitude = 3.0;

  float fov_y = float(M_PI) / 4.0f;
  mat4 view = mat4::eye();
  mat4 projection = mat4::eye();

  dvec3 eye_position() const {
    double r = 1.0 + altitude;
    double cl = cos(latitude), sl = sin(latitude);
    double cp = cos(longitude), sp = sin(longitude);
    return dvec3{r * cl * cp, r * sl, -r * cl * sp};
  }

  dvec3 look_at_point() const {
    double cl = cos(latitude), sl = sin(latitude);
    double cp = cos(longitude), sp = sin(longitude);
    return dvec3{cl * cp, sl, -cl * sp};
  }

  void update_matrices() {
    dvec3 eye = eye_position();
    double eye_len = norm(eye);
    dvec3 fwd_d = -eye / eye_len;
    dvec3 world_up = {0.0, 1.0, 0.0};

    double up_dot = std::abs(dot(fwd_d, world_up));
    if (up_dot > 0.999)
      world_up = dvec3{0.0, 0.0, -1.0};

    dvec3 right_d = normalize(cross(fwd_d, world_up));
    dvec3 up_d = cross(right_d, fwd_d);

    view = mat4::eye();
    view(0, 0) = float(right_d[0]);
    view(0, 1) = float(right_d[1]);
    view(0, 2) = float(right_d[2]);
    view(1, 0) = float(up_d[0]);
    view(1, 1) = float(up_d[1]);
    view(1, 2) = float(up_d[2]);
    view(2, 0) = float(-fwd_d[0]);
    view(2, 1) = float(-fwd_d[1]);
    view(2, 2) = float(-fwd_d[2]);
    view(0, 3) = 0.0f;
    view(1, 3) = 0.0f;
    view(2, 3) = 0.0f;
    view(3, 3) = 1.0f;

    float near = fmaxf(0.0001f, float(altitude) * 0.01f);
    float far = 2.0f * float(1.0 + altitude) + 1.0f;
    projection = perspective(fov_y, 640.0f / 480.0f, length{near}, length{far});
  }

  void on_enter() override { update_matrices(); }

  void orbit(float jx, float jy, float dt) {
    if (fabsf(jx) < 0.08f)
      jx = 0.0f;
    if (fabsf(jy) < 0.08f)
      jy = 0.0f;
    if (jx == 0.0f && jy == 0.0f)
      return;

    double speed = fmax(0.02, altitude) * 0.8;
    longitude += jx * speed * dt / fmax(0.1, cos(latitude));
    latitude -= jy * speed * dt;

    latitude = fmax(-1.48, fmin(1.48, latitude));
    if (longitude > M_PI)
      longitude -= 2.0 * M_PI;
    if (longitude < -M_PI)
      longitude += 2.0 * M_PI;

    update_matrices();
  }

  void zoom(double amount) {
    if (amount == 0.0)
      return;

    altitude = fmax(0.01, fmin(10.0, altitude + amount));
    update_matrices();
  }

  void on_joystick(float jx, float jy, float, float) override {
    orbit(jx, jy, 1.0f / 60.0f);
  }

  void on_trigger(float lt, float rt) override {
    if (lt < 0.02f && rt < 0.02f)
      return;

    zoom(double(lt - rt) * altitude * 1.5 * (1.0 / 60.0));
  }

  void on_keyboard(const keyboard_input_state &keyboard) override {
    float jx = 0.0f;
    float jy = 0.0f;

    if (keyboard.is_down(KBD_KEY_LEFT) || keyboard.is_down(KBD_KEY_A))
      jx -= 1.0f;
    if (keyboard.is_down(KBD_KEY_RIGHT) || keyboard.is_down(KBD_KEY_D))
      jx += 1.0f;
    if (keyboard.is_down(KBD_KEY_UP) || keyboard.is_down(KBD_KEY_W))
      jy -= 1.0f;
    if (keyboard.is_down(KBD_KEY_DOWN) || keyboard.is_down(KBD_KEY_S))
      jy += 1.0f;

    orbit(jx, jy, 1.0f / 60.0f);

    double zoom_step = altitude * 1.5 * (1.0 / 60.0);
    if (keyboard.is_down(KBD_KEY_MINUS) ||
        keyboard.is_down(KBD_KEY_PAD_MINUS) || keyboard.is_down(KBD_KEY_PGDOWN))
      zoom(zoom_step);
    if (keyboard.is_down(KBD_KEY_PLUS) || keyboard.is_down(KBD_KEY_PAD_PLUS) ||
        keyboard.is_down(KBD_KEY_PGUP))
      zoom(-zoom_step);
  }

  void on_mouse(const mouse_input_state &mouse) override {
    if (mouse.buttons & MOUSE_LEFTBUTTON)
      orbit(-mouse.dx * mouse_orbit_sensitivity,
            -mouse.dy * mouse_orbit_sensitivity, 1.0f / 60.0f);

    if (mouse.dz != 0)
      zoom(double(mouse.dz) * fmax(0.02, altitude) * mouse_zoom_sensitivity);
  }
};

} // namespace dcearth

#endif
