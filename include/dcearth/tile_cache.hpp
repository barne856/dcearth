#ifndef DCEARTH_TILE_CACHE_HPP
#define DCEARTH_TILE_CACHE_HPP

#include "dcearth/tile_id.hpp"
#include <array>
#include <dc/pvr.h>

namespace dcearth {

class tile_cache {
public:
  static constexpr int MAX_SLOTS = 32;
  static constexpr size_t TILE_BYTES = 256 * 256 * 2;

  struct slot {
    pvr_ptr_t vram = nullptr;
    tile_id tile = {0, 0, 0};
    uint32_t last_used_frame = 0;
    bool occupied = false;
    bool pinned = false;
    pvr_poly_hdr_t header = {};
  };

  void init() {
    num_slots_ = 0;
    for (int i = 0; i < MAX_SLOTS; i++) {
      slots_[i].vram = pvr_mem_malloc(TILE_BYTES);
      if (!slots_[i].vram)
        break;
      slots_[i].occupied = false;
      slots_[i].pinned = false;
      num_slots_++;
    }
    frame_ = 0;
  }

  void destroy() {
    for (int i = 0; i < num_slots_; i++) {
      if (slots_[i].vram) {
        pvr_mem_free(slots_[i].vram);
        slots_[i].vram = nullptr;
      }
      slots_[i].occupied = false;
    }
  }

  int lookup(tile_id id) {
    uint64_t key = tile_key(id);
    for (int i = 0; i < num_slots_; i++) {
      if (slots_[i].occupied && tile_key(slots_[i].tile) == key) {
        slots_[i].last_used_frame = frame_;
        return i;
      }
    }
    return -1;
  }

  bool is_cached(tile_id id) {
    uint64_t key = tile_key(id);
    for (int i = 0; i < num_slots_; i++) {
      if (slots_[i].occupied && tile_key(slots_[i].tile) == key)
        return true;
    }
    return false;
  }

  pvr_ptr_t vram_of(int idx) { return slots_[idx].vram; }
  pvr_poly_hdr_t *header_of(int idx) { return &slots_[idx].header; }

  int find_free_slot() {
    for (int i = 0; i < num_slots_; i++) {
      if (!slots_[i].occupied)
        return i;
    }
    return -1;
  }

  int evict_lru() {
    int best = -1;
    uint32_t oldest = UINT32_MAX;
    for (int i = 0; i < num_slots_; i++) {
      // +1: poll_and_upload runs before traversal, so protect last frame's
      // tiles
      if (slots_[i].occupied && !slots_[i].pinned &&
          slots_[i].last_used_frame + 1 < frame_ &&
          slots_[i].last_used_frame < oldest) {
        oldest = slots_[i].last_used_frame;
        best = i;
      }
    }
    if (best >= 0)
      slots_[best].occupied = false;
    return best;
  }

  void insert(int idx, tile_id id, const uint16_t *rgb565_data,
              bool pin = false) {
    auto &s = slots_[idx];
    pvr_txr_load_ex((void *)rgb565_data, s.vram, 256, 256, PVR_TXRLOAD_16BPP);
    s.tile = id;
    s.last_used_frame = frame_;
    s.occupied = true;
    s.pinned = pin;

    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY,
                     PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED, 256, 256, s.vram,
                     PVR_FILTER_BILINEAR);
    cxt.gen.shading = PVR_SHADE_GOURAUD;
    pvr_poly_compile(&s.header, &cxt);
  }

  void advance_frame() { frame_++; }
  int num_slots() const { return num_slots_; }

private:
  std::array<slot, MAX_SLOTS> slots_;
  int num_slots_ = 0;
  uint32_t frame_ = 0;
};

} // namespace dcearth

#endif
