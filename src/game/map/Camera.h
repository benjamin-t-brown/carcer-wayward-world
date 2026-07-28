#pragma once

namespace model {
struct MapInstance;
}

namespace game {

struct CameraPos {
  int camX = 0;
  int camY = 0;
};

// Center camera on target tile. Does not clamp — map may scroll past its edges
// so the target stays centered (empty space outside the map is allowed).
CameraPos computeCameraFollow(int targetTileX,
                              int targetTileY,
                              const model::MapInstance& map,
                              int viewW,
                              int viewH);

} // namespace game
