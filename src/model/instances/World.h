#pragma once

#include "model/Combat.h"
#include "model/instances/MapInstance.h"
#include "model/templates/UtilityTypes.h"
#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

enum class CameraMode { Follow, Aiming, Dragging, Controlled };

enum class WorldActionMode { NONE, EXAMINE, TALK };

// Map-space hit feedback: splash animation plus a numeric label (not UI floating text).
struct DamageParticle {
  bmin::String animationName;
  int tileX = 0;
  int tileY = 0;
  int value = 0;
  TimerStruct lifetime;
};

struct CameraInfo {
  int camX = 0; // map pixel space
  int camY = 0;
  CameraMode cameraMode = CameraMode::Follow;
  // empty = auto-resolve to current party member avatar when cameraMode is Follow
  bmin::String cameraFollowCharacterId;
  int viewW = 0; // MapView content size in map-pixel space (unscaled)
  int viewH = 0;
};

struct World {
  bmin::String name;
  MapInstance currentMap;
  CameraInfo camera;
  WorldActionMode actionMode = WorldActionMode::NONE;
  // Meaningful only when actionMode != NONE (Examine / Talk aim cursor).
  std::optional<TileXY> actionAimTile;
  Combat combat;
  bmin::DynArray<DamageParticle> damageParticles;
  // Monotonic counter of player movement ticks (world steps + combat rounds × 4).
  int playerMovementCount = 0;
};

} // namespace model
