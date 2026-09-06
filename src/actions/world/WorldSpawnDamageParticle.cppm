module;
#include <utility>

export module carcer.actions.world:WorldSpawnDamageParticle;
export import carcer.state;

export {

namespace state::actions {

class WorldSpawnDamageParticle : public AbstractAction {
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
      : animationName(_animationName),
        text(_text),
        tileX(_tileX),
        tileY(_tileY),
        lifetimeMs(_lifetimeMs) {}
};

} // namespace state::actions

} // export
