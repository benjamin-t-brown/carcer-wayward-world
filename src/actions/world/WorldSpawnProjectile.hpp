#pragma once

#include "bmin/String.h"
#include "model/instances/World.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/UtilityTypes.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class WorldSpawnProjectile : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnProjectile; }
  bmin::String animationName;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  int travelMs = 0;
  // affects the "height" of the projectile, the y offset of the projectile on a sin wave
  // from start to end
  // - NONE means lerp directly to target
  // - SHORT means offset by half tile height (scaled)
  // - MEDIUM means offset by full tile height (scaled)
  // - TALL means offset by 2x tile height (scaled)
  model::ProjectilePath projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;

  void act() override {
    if (!state) {
      return;
    }

    model::WorldProjectile projectile;
    projectile.animationName = animationName;
    projectile.fromTileX = fromTileX;
    projectile.fromTileY = fromTileY;
    projectile.toTileX = toTileX;
    projectile.toTileY = toTileY;
    projectile.projectilePath = projectilePath;
    // projectile.anim = sdl2w::Animation::create(animationName);
    model::timerStructStart(projectile.travel, travelMs);
    state->world.activeMap.projectiles.pushBack(std::move(projectile));
  }

public:
  WorldSpawnProjectile(const bmin::String& _animationName,
                       float _fromTileX,
                       float _fromTileY,
                       float _toTileX,
                       float _toTileY,
                       int _travelMs,
                       model::ProjectilePath _projectilePath)
      : animationName(_animationName),
        fromTileX(_fromTileX),
        fromTileY(_fromTileY),
        toTileX(_toTileX),
        toTileY(_toTileY),
        travelMs(_travelMs),
        projectilePath(_projectilePath) {}
};

} // namespace actions

} // namespace state
