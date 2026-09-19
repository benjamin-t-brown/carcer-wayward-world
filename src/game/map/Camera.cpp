#include "game/map/Camera.h"
#include "model/instances/World.hpp"

namespace game {

CameraPos computeCameraFollow(int targetTileX, int targetTileY, int viewW, int viewH) {
  auto spriteW = kCameraTileWidth;
  auto spriteH = kCameraTileHeight;
  // Free scroll: allow negative / past-edge cam so the target stays centered.
  auto camX = targetTileX * spriteW - viewW / 2 + spriteW / 2;
  auto camY = targetTileY * spriteH - viewH / 2 + spriteH / 2;
  return CameraPos{.camX = camX, .camY = camY};
}

void snapCameraToTile(model::CameraInfo& camera, int targetTileX, int targetTileY) {
  if (camera.viewW <= 0 || camera.viewH <= 0) {
    return;
  }
  const auto cam = computeCameraFollow(targetTileX, targetTileY, camera.viewW, camera.viewH);
  camera.camX = cam.camX;
  camera.camY = cam.camY;
}

} // namespace game
