#pragma once

namespace model {
struct World;
} // namespace model

namespace game {

struct CombatRunner {
  const model::World* world;

  CombatRunner(const model::World* world);
  ~CombatRunner();

  void startCombat();
  void startTurn();
  void endTurn();
  void endCombat();
};

} // namespace game