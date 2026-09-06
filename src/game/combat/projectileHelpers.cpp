module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.game.combat;
import bmin.containers;
import carcer.data;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {

bmin::String getProjectileFacingSuffix(int dx, int dy) {
  if (dx == 0 && dy == 0) {
    return "_s";
  }
  if (dx == 0) {
    return dy < 0 ? "_n" : "_s";
  }
  if (dy == 0) {
    return dx < 0 ? "_w" : "_e";
  }
  if (dy < 0) {
    return dx < 0 ? "_nw" : "_ne";
  }
  return dx < 0 ? "_sw" : "_se";
}

int getProjectileTravelDurationMs(model::ProjectilePath path) {
  int durationMs = 150;
  switch (path) {
  case model::ProjectilePath::PROJECTILE_PATH_SHORT:
    durationMs = 300;
    break;
  case model::ProjectilePath::PROJECTILE_PATH_MEDIUM:
    durationMs = 500;
    break;
  case model::ProjectilePath::PROJECTILE_PATH_TALL:
    durationMs = 750;
    break;
  case model::ProjectilePath::PROJECTILE_PATH_NONE:
    break;
  }
  return durationMs;
}

} // namespace game
