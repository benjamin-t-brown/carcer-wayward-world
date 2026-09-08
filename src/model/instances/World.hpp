#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include "model/Combat.h"
#include "model/instances/MapInstance.h"
#include "model/templates/AbilityTypes.h"
#include "model/templates/UtilityTypes.h"
#include "compat/Sdl2wAnimation.hpp"
#include <optional>

namespace model {

enum class CameraMode { Follow, Aiming, Dragging, Controlled };

enum class WorldActionMode { NONE, EXAMINE, TALK, SPELL };

// Map-space hit feedback: splash animation plus a numeric label (not UI floating text).
struct DamageParticle {
  bmin::String animationName;
  std::optional<sdl2w::Animation> animation;
  bmin::String text;
  int tileX = 0;
  int tileY = 0;
  TimerStruct lifetime;
};

// Traveling combat projectile (tile-space lerp from caster to zone origin).
struct WorldProjectile {
  bmin::String animationName;
  bmin::String text;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  ProjectilePath projectilePath = ProjectilePath::PROJECTILE_PATH_NONE;
  TimerStruct travel;
  int yOffset = 0;
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
  bmin::DynArray<WorldProjectile> projectiles;
};

struct World {
  ActiveMap activeMap;

  CameraInfo camera;
  WorldActionMode actionMode = WorldActionMode::NONE;
  // Meaningful only when actionMode != NONE (Examine / Talk / Spell aim cursor).
  std::optional<TileXY> actionAimTile;
  // Meaningful only when actionMode == SPELL (spell template name, e.g. "FLAME").
  bmin::String pendingSpellId;
  bmin::String pendingChId;

  // True while town enemy AI is resolving (seek / melee swing / particles).
  // Blocks player movement and world actions until the timed sequence finishes.
  bool resolvingTownEnemyAi = false;

  Combat combat;
};

} // namespace model
