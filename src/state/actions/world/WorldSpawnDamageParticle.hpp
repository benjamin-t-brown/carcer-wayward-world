#pragma once

#include "model/instances/World.h"
#include "model/templates/UtilityTypes.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"

namespace state {

namespace actions {

class WorldSpawnDamageParticle : public AbstractAction {
  bmin::String animationName;
  int tileX = 0;
  int tileY = 0;
  int value = 0;
  int lifetimeMs = 0;

  void act() override {
    if (!state) {
      return;
    }

    model::DamageParticle particle;
    particle.animationName = animationName;
    particle.tileX = tileX;
    particle.tileY = tileY;
    particle.value = value;
    model::timerStructStart(particle.lifetime, lifetimeMs);
    state->world.damageParticles.pushBack(std::move(particle));
  }

public:
  WorldSpawnDamageParticle(bmin::String _animationName,
                           int _tileX,
                           int _tileY,
                           int _value,
                           int _lifetimeMs)
      : animationName(std::move(_animationName)),
        tileX(_tileX),
        tileY(_tileY),
        value(_value),
        lifetimeMs(_lifetimeMs) {}
};

} // namespace actions

} // namespace state
