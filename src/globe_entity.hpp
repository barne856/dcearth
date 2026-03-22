#ifndef DCEARTH_GLOBE_ENTITY_HPP
#define DCEARTH_GLOBE_ENTITY_HPP

#include "camera_entity.hpp"
#include "dcearth/entity.hpp"
#include "dcearth/tile_cache.hpp"
#include "dcearth/tile_loader.hpp"
#include "dcearth/tile_material.hpp"
#include "dcearth/tile_mesh.hpp"
#include <algorithm>
#include <cmath>
#include <dc/pvr.h>
#include <vector>

namespace dcearth {

struct visible_tile {
  tile_id id;
  int cache_idx;
  float u_off, v_off, u_scale, v_scale;
};

class globe_entity : public entity<globe_entity> {
public:
  tile_material mat;
  const orbital_camera *cam = nullptr;
  int month = 1;
  int max_lod = 5;
  float lod_error_threshold = 2.0f;

  void on_enter() override {
    cache_.init();
    loader_.init(&cache_, &source_, month);

    uint32_t cols = tile_cols(0);
    uint32_t rows = tile_rows(0);
    for (uint32_t x = 0; x < cols; x++)
      for (uint32_t y = 0; y < rows; y++)
        loader_.load_sync({0, x, y}, true);
  }

  void on_exit() override {
    loader_.shutdown();
    cache_.destroy();
  }

  void update(float) {
    cache_.advance_frame();
    loader_.poll_and_upload();

    visible_.clear();
    uint32_t cols = tile_cols(0);
    uint32_t rows = tile_rows(0);
    for (uint32_t x = 0; x < cols; x++)
      for (uint32_t y = 0; y < rows; y++)
        traverse({0, x, y}, *cam);

    loader_.sort_queue();

    std::sort(visible_.begin(), visible_.end(),
              [](const visible_tile &a, const visible_tile &b) {
                return a.cache_idx < b.cache_idx;
              });
  }

  void render(float) {
    mat4 vp = cam->projection * cam->view;
    dvec3 cam_pos = cam->eye_position();
    int last_cache_idx = -1;
    for (auto &vt : visible_) {
      if (vt.cache_idx != last_cache_idx) {
        mat.set_header(cache_.header_of(vt.cache_idx));
        mat.bind();
        last_cache_idx = vt.cache_idx;
      }
      tile_.generate(vt.id, vt.u_off, vt.v_off, vt.u_scale, vt.v_scale,
                     cam_pos);
      tile_.draw(&mat, vp);
    }
  }

private:
  bool is_visible(tile_id t, const orbital_camera &cam) {
    if (t.lod < 1)
      return true;

    dvec3 cam_pos = cam.eye_position();
    double cam_dist = norm(cam_pos);
    dvec3 cam_dir = cam_pos / cam_dist;
    dvec3 tc = center_on_sphere(t);

    double d = dot(tc, cam_dir);
    double horizon_cos = 1.0 / cam_dist;
    double tile_half = M_PI * 1.5 / double(1u << t.lod);
    double margin = (tile_half < M_PI / 2.0) ? sin(tile_half) : 1.0;

    if (d <= horizon_cos - margin)
      return false;

    dvec3 look = cam.look_at_point();
    double view_dot = dot(tc, look);
    double cone_cos = cos(double(cam.fov_y) * 0.75 + tile_half);
    if (view_dot < cone_cos)
      return false;

    return true;
  }

  float screen_error(tile_id t, const orbital_camera &cam) {
    dvec3 cam_pos = cam.eye_position();
    dvec3 tc = center_on_sphere(t);
    dvec3 diff = tc - cam_pos;
    float dist = float(norm(diff));
    if (dist < 0.0001f)
      dist = 0.0001f;

    float tile_angular_size = float(M_PI) / float(1u << t.lod);
    float screen_height = 480.0f;
    float projected_pixels =
        (tile_angular_size / dist) * (screen_height / cam.fov_y);

    return projected_pixels / 256.0f;
  }

  float priority_of(tile_id t, const orbital_camera &cam) {
    dvec3 look = cam.look_at_point();
    dvec3 tc = center_on_sphere(t);
    double angular_dist = acos(fmin(1.0, dot(look, tc)));
    return float(angular_dist) + float(t.lod) * 0.1f;
  }

  void emit_tile(tile_id t, int cache_idx) {
    visible_.push_back({t, cache_idx, 0.0f, 0.0f, 1.0f, 1.0f});
  }

  bool try_emit_cached_children(tile_id t, const orbital_camera &cam) {
    auto kids = children(t);
    bool any_cached = false;
    for (auto &child : kids) {
      if (cache_.is_cached(child)) {
        any_cached = true;
        break;
      }
    }
    if (!any_cached)
      return false;

    for (auto &child : kids) {
      int idx = cache_.lookup(child);
      if (idx >= 0)
        emit_tile(child, idx);
      else
        emit_with_ancestor_fallback(child, cam);
    }
    return true;
  }

  void emit_with_ancestor_fallback(tile_id t, const orbital_camera &cam) {
    loader_.request(t, priority_of(t, cam));

    tile_id ancestor = t;
    int ancestor_idx = -1;
    while (ancestor.lod > 0) {
      ancestor = parent(ancestor);
      ancestor_idx = cache_.lookup(ancestor);
      if (ancestor_idx >= 0)
        break;
    }
    if (ancestor_idx < 0)
      return;

    int lod_diff = t.lod - ancestor.lod;
    float scale = 1.0f / float(1u << lod_diff);
    float u_off = float(t.x % (1u << lod_diff)) * scale;
    float v_off = float(t.y % (1u << lod_diff)) * scale;

    visible_.push_back({t, ancestor_idx, u_off, v_off, scale, scale});
  }

  void traverse(tile_id t, const orbital_camera &cam) {
    if (!is_visible(t, cam))
      return;

    // LOD 0 tiles span a hemisphere so screen_error is unreliable (center is on
    // far side)
    if (t.lod < 1 && t.lod < max_lod) {
      auto kids = children(t);
      for (auto &child : kids)
        traverse(child, cam);
      return;
    }

    float error = screen_error(t, cam);
    if (t.lod >= max_lod || error < lod_error_threshold) {
      int idx = cache_.lookup(t);
      if (idx >= 0) {
        emit_tile(t, idx);
      } else {
        if (t.lod < max_lod && try_emit_cached_children(t, cam))
          return;
        emit_with_ancestor_fallback(t, cam);
      }
      return;
    }

    auto kids = children(t);
    for (auto &child : kids)
      traverse(child, cam);
  }

  tile_cache cache_;
  tile_loader loader_;
  tile_mesh tile_;
  file_tile_source source_{"/pc/var/home/core/dev/dcearth/tiles/bluemarble"};
  std::vector<visible_tile> visible_;
};

} // namespace dcearth

#endif
