#include "game/map/Camera.h"
#include "model/instances/MapInstance.h"

namespace game {

CameraPos computeCameraFollow(int targetTileX,
                              int targetTileY,
                              const model::MapInstance& map,
                              int viewW,
                              int viewH) {
  auto spriteW = map.spriteWidth > 0 ? map.spriteWidth : 28;
  auto spriteH = map.spriteHeight > 0 ? map.spriteHeight : 32;
  // Free scroll: allow negative / past-edge cam so the target stays centered.
  auto camX = targetTileX * spriteW - viewW / 2 + spriteW / 2;
  auto camY = targetTileY * spriteH - viewH / 2 + spriteH / 2;
  return CameraPos{.camX = camX, .camY = camY};
}

} // namespace game
