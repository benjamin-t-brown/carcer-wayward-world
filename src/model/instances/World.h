#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "model/Combat.h"
#include "model/instances/MapInstance.h"
#include "model/templates/UtilityTypes.h"
#include <optional>

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

struct ActiveMap {
  bmin::String gridId;
  int mapLayer = 0;
  bmin::DynArray<CharacterInstance> characters;
  bmin::DynArray<ItemInstance> items;
  // bmin::DynArray<TileField> fields;
  bmin::DynArray<DamageParticle> damageParticles;
};

struct World {
  ActiveMap activeMap;

  CameraInfo camera;
  WorldActionMode actionMode = WorldActionMode::NONE;
  // Meaningful only when actionMode != NONE (Examine / Talk aim cursor).
  std::optional<TileXY> actionAimTile;

  Combat combat;
};

} // namespace model
