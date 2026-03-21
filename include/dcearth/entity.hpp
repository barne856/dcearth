#ifndef DCEARTH_ENTITY_HPP
#define DCEARTH_ENTITY_HPP

#include <cstdint>
#include <memory>
#include <vector>

namespace dcearth {

template <typename T> class render_system {
  public:
    virtual ~render_system() = default;
    virtual void render(float dt, T &entity) = 0;
};

template <typename T> class physics_system {
  public:
    virtual ~physics_system() = default;
    virtual void update(float dt, T &entity) = 0;
};

template <typename T> class controls_system {
  public:
    virtual ~controls_system() = default;
    virtual bool on_button(uint32_t buttons, T &entity) { return false; }
    virtual bool on_joystick(float jx, float jy, float j2x, float j2y, T &entity) { return false; }
    virtual bool on_trigger(float lt, float rt, T &entity) { return false; }
};

class entity_base {
  public:
    virtual ~entity_base() = default;
    virtual void on_enter() {}
    virtual void on_exit() {}
    virtual void on_render(float dt) {}
    virtual void on_update(float dt) {}
    virtual void on_button(uint32_t buttons) {}
    virtual void on_joystick(float jx, float jy, float j2x, float j2y) {}
    virtual void on_trigger(float lt, float rt) {}
};

template <typename T> class entity : public entity_base {
  public:
    template <typename U, typename... Args> void attach_render_system(Args &&...args) {
        render_systems_.emplace_back(std::make_unique<U>(std::forward<Args>(args)...));
    }
    template <typename U, typename... Args> void attach_physics_system(Args &&...args) {
        physics_systems_.emplace_back(std::make_unique<U>(std::forward<Args>(args)...));
    }
    template <typename U, typename... Args> void attach_controls_system(Args &&...args) {
        controls_systems_.emplace_back(std::make_unique<U>(std::forward<Args>(args)...));
    }

    void render(float dt) {}
    void update(float dt) {}
    void button(uint32_t buttons) {}
    void joystick(float jx, float jy, float j2x, float j2y) {}
    void trigger(float lt, float rt) {}

    void on_render(float dt) override {
        for (auto &sys : render_systems_)
            sys->render(dt, self());
        self().render(dt);
    }

    void on_update(float dt) override {
        for (auto &sys : physics_systems_)
            sys->update(dt, self());
        self().update(dt);
    }

    void on_button(uint32_t buttons) override {
        for (auto it = controls_systems_.rbegin(); it != controls_systems_.rend(); ++it)
            if ((*it)->on_button(buttons, self()))
                return;
        self().button(buttons);
    }

    void on_joystick(float jx, float jy, float j2x, float j2y) override {
        for (auto it = controls_systems_.rbegin(); it != controls_systems_.rend(); ++it)
            if ((*it)->on_joystick(jx, jy, j2x, j2y, self()))
                return;
        self().joystick(jx, jy, j2x, j2y);
    }

    void on_trigger(float lt, float rt) override {
        for (auto it = controls_systems_.rbegin(); it != controls_systems_.rend(); ++it)
            if ((*it)->on_trigger(lt, rt, self()))
                return;
        self().trigger(lt, rt);
    }

  private:
    T &self() { return static_cast<T &>(*this); }
    std::vector<std::unique_ptr<render_system<T>>> render_systems_;
    std::vector<std::unique_ptr<physics_system<T>>> physics_systems_;
    std::vector<std::unique_ptr<controls_system<T>>> controls_systems_;
};

} // namespace dcearth

#endif
