#pragma once

#include "bmin/String.h"
#include "model/instances/World.hpp"
#include "model/templates/UtilityTypes.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class WorldSpawnDamageParticle : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnDamageParticle; }
  bmin::String animationName;
  bmin::String text;
  int tileX = 0;
  int tileY = 0;
  int lifetimeMs = 0;

  void act() override {
    if (!state) {
      return;
    }

    model::DamageParticle particle;
    particle.animationName = animationName;
    particle.tileX = tileX;
    particle.tileY = tileY;
    particle.text = text;
    model::timerStructStart(particle.lifetime, lifetimeMs);
    state->world.activeMap.damageParticles.pushBack(std::move(particle));
  }

public:
  WorldSpawnDamageParticle(const bmin::String& _animationName,
                           const bmin::String& _text,
                           int _tileX,
                           int _tileY,
                           int _lifetimeMs)
      : animationName((_animationName)),
        text((_text)),
        tileX(_tileX),
        tileY(_tileY),
        lifetimeMs(_lifetimeMs) {}
};

} // namespace actions

} // namespace state
