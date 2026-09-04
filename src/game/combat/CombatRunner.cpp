module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.game.combat;
import carcer.model.instances.World;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {

CombatRunner::CombatRunner(const model::World* world) : world(world) {}

CombatRunner::~CombatRunner() {}

// void CombatRunner::startCombat() { combat = model::createCombatFromWorld(*world); }

} // namespace game
