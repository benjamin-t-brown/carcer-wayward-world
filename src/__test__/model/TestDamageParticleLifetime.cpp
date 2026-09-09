// Regression test for the world-simulation / platform-output split (Phase 6).
//
// A damage particle's lifetime is world simulation and must advance during a
// headless worldUpdate (no window), so expired particles are removed instead of
// leaking forever. Before the split, updateDamageParticles returned early when
// window == nullptr and the lifetime never advanced.
#include "state/WorldUpdater.h"
#include "model/instances/World.hpp"
#include "model/templates/UtilityTypes.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include <iostream>

namespace {

bool expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
  }
  return condition;
}

model::DamageParticle makeParticle(int lifetimeMs) {
  model::DamageParticle particle;
  particle.animationName = "anim_hit";
  particle.text = "10";
  particle.lifetime = model::TimerStruct(lifetimeMs);
  return particle;
}

} // namespace

int main() {
  bool ok = true;

  // Expired particles are removed by a headless update.
  {
    state::StateManager stateManager;
    auto& state = stateManager.getState();
    state.world.activeMap.damageParticles.pushBack(makeParticle(100));

    ok = expect(state.world.activeMap.damageParticles.size() == 1,
                "particle seeded before update") && ok;

    state::worldUpdate(nullptr, stateManager, 1000);
    ok = expect(state.world.activeMap.damageParticles.empty(),
                "headless worldUpdate ages and removes an expired particle") && ok;
  }

  // A particle whose lifetime has not elapsed survives, but its lifetime still
  // advances (no visual animation is created without a window).
  {
    state::StateManager stateManager;
    auto& state = stateManager.getState();
    state.world.activeMap.damageParticles.pushBack(makeParticle(1000));

    state::worldUpdate(nullptr, stateManager, 100);
    ok = expect(state.world.activeMap.damageParticles.size() == 1,
                "unexpired particle survives a headless update") && ok;
    if (!state.world.activeMap.damageParticles.empty()) {
      const auto& particle = state.world.activeMap.damageParticles[0];
      ok = expect(particle.lifetime.t == 100,
                  "lifetime advances even without a window") && ok;
      ok = expect(!particle.animation.has_value(),
                  "no platform animation is created headless") && ok;
    }
  }

  return ok ? 0 : 1;
}
