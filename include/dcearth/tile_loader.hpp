#ifndef DCEARTH_TILE_LOADER_HPP
#define DCEARTH_TILE_LOADER_HPP

#include "dcearth/tile_cache.hpp"
#include "dcearth/tile_source.hpp"
#include <algorithm>
#include <cstdlib>
#include <kos/mutex.h>
#include <kos/thread.h>
#include <malloc.h>
#include <vector>

namespace dcearth {

class tile_loader {
public:
  void init(tile_cache *cache, tile_source *source, int month) {
    cache_ = cache;
    source_ = source;
    month_ = month;
    shutdown_ = false;
    decode_ready_ = false;
    has_work_ = false;
    decode_buf_ =
        static_cast<uint16_t *>(memalign(32, 256 * 256 * sizeof(uint16_t)));

    mutex_init(&mtx_, MUTEX_TYPE_NORMAL);

    kthread_attr_t attr = {};
    attr.create_detached = false;
    attr.stack_size = 64 * 1024;
    attr.label = "tile_loader";
    thread_ = thd_create_ex(&attr, loader_thread, this);
  }

  void shutdown() {
    mutex_lock(&mtx_);
    shutdown_ = true;
    mutex_unlock(&mtx_);

    if (thread_) {
      thd_join(thread_, nullptr);
      thread_ = nullptr;
    }

    mutex_destroy(&mtx_);

    free(decode_buf_);
    decode_buf_ = nullptr;
  }

  void request(tile_id id, float priority) {
    if (cache_->is_cached(id))
      return;

    mutex_lock(&mtx_);
    for (auto &r : pending_) {
      if (r.id == id) {
        r.priority = fminf(r.priority, priority);
        mutex_unlock(&mtx_);
        return;
      }
    }
    pending_.push_back({id, priority});
    has_work_ = true;
    mutex_unlock(&mtx_);
  }

  void sort_queue() {
    mutex_lock(&mtx_);
    pending_.erase(std::remove_if(pending_.begin(), pending_.end(),
                                  [this](const request_entry &r) {
                                    return cache_->is_cached(r.id);
                                  }),
                   pending_.end());
    std::sort(
        pending_.begin(), pending_.end(),
        [](const auto &a, const auto &b) { return a.priority < b.priority; });
    has_work_ = !pending_.empty();
    mutex_unlock(&mtx_);
  }

  bool poll_and_upload() {
    mutex_lock(&mtx_);
    if (!decode_ready_) {
      mutex_unlock(&mtx_);
      return false;
    }

    tile_id t = decoded_tile_;
    decode_ready_ = false;
    mutex_unlock(&mtx_);

    if (cache_->lookup(t) >= 0)
      return false;

    int idx = cache_->find_free_slot();
    if (idx < 0)
      idx = cache_->evict_lru();
    if (idx < 0)
      return false;

    cache_->insert(idx, t, decode_buf_);
    return true;
  }

  bool load_sync(tile_id id, bool pin = false) {
    if (!source_->fetch(id, month_, decode_buf_, 256))
      return false;

    int idx = cache_->find_free_slot();
    if (idx < 0)
      idx = cache_->evict_lru();
    if (idx < 0)
      return false;

    cache_->insert(idx, id, decode_buf_, pin);
    return true;
  }

private:
  struct request_entry {
    tile_id id;
    float priority;
  };

  static void *loader_thread(void *arg) {
    auto *self = static_cast<tile_loader *>(arg);
    self->thread_loop();
    return nullptr;
  }

  void thread_loop() {
    while (true) {
      mutex_lock(&mtx_);
      if (shutdown_) {
        mutex_unlock(&mtx_);
        return;
      }

      if (decode_ready_ || !has_work_ || pending_.empty()) {
        mutex_unlock(&mtx_);
        thd_pass();
        continue;
      }

      tile_id target = pending_.front().id;
      int month = month_;
      pending_.erase(pending_.begin());
      has_work_ = !pending_.empty();
      mutex_unlock(&mtx_);

      if (source_->fetch(target, month, decode_buf_, 256)) {
        mutex_lock(&mtx_);
        decoded_tile_ = target;
        decode_ready_ = true;
        mutex_unlock(&mtx_);
      }
    }
  }

  tile_cache *cache_ = nullptr;
  tile_source *source_ = nullptr;
  int month_ = 1;

  mutex_t mtx_;
  bool shutdown_ = false;

  std::vector<request_entry> pending_;
  bool has_work_ = false;

  uint16_t *decode_buf_ = nullptr;
  tile_id decoded_tile_ = {};
  bool decode_ready_ = false;

  kthread_t *thread_ = nullptr;
};

} // namespace dcearth

#endif
