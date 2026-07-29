#include "game/map/Camera.h"

namespace game {

CameraPos computeCameraFollow(int targetTileX, int targetTileY, int viewW, int viewH) {
  auto spriteW = 28;
  auto spriteH = 32;
  // Free scroll: allow negative / past-edge cam so the target stays centered.
  auto camX = targetTileX * spriteW - viewW / 2 + spriteW / 2;
  auto camY = targetTileY * spriteH - viewH / 2 + spriteH / 2;
  return CameraPos{.camX = camX, .camY = camY};
}

} // namespace game
