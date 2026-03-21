#include <kos.h>

#include "dcearth/renderer.hpp"
#include "main_scene.hpp"

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char *argv[]) {
  auto &ren = dcearth::renderer::get();
  dcearth::main_scene scene;
  ren.set_scene(&scene);
  ren.run();
}
