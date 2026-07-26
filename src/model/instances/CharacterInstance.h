#pragma once

#include "bmin/String.h"

namespace model {

// Map-entity character (player avatar, future NPCs). Distinct from CharacterPlayer
// (party chrome). Position is tile coords in map space, same as TileInstance.
struct CharacterInstance {
  bmin::String id;
  bmin::String name;
  bmin::String templateName;
  int x = 0;
  int y = 0;
  // Combat runtime (meaningful while world.combat.active).
  int currentAp = 0;
  int currentHp = 0; // enemies only; party HP lives on CharacterPlayer
  int spriteIndex = 0;
};

} // namespace model
