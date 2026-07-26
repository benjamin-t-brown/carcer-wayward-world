#include "game/combat/CombatRunner.h"

namespace game {

CombatRunner::CombatRunner(const model::World* world) : world(world) {}

CombatRunner::~CombatRunner() {}

// void CombatRunner::startCombat() { combat = model::createCombatFromWorld(*world); }

} // namespace game