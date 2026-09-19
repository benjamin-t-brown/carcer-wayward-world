#pragma once

namespace model {
struct CameraInfo;
struct MapInstance;
}

namespace game {

struct CameraPos {
  int camX = 0;
  int camY = 0;
};

constexpr int kCameraTileWidth = 28;
constexpr int kCameraTileHeight = 32;

// Center camera on target tile. Does not clamp — map may scroll past its edges
// so the target stays centered (empty space outside the map is allowed).
CameraPos computeCameraFollow(int targetTileX, int targetTileY, int viewW, int viewH);

// No-op when viewW or viewH is not positive.
void snapCameraToTile(model::CameraInfo& camera, int targetTileX, int targetTileY);

} // namespace game
