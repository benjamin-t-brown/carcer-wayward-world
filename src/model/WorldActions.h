#pragma once

namespace model {

// World action types enum
enum class WorldActionType {
  JUMP,
  ABILITY,
  STATUS,
  TALK,
  END_FIGHT,
  GET,
  MAP,
  MAP_OUTDOOR,
  SNEAK,
  START_FIGHT,
  UNLOCK,
  EXAMINE,
  INVENTORY,
  SHOOT,
  DEFEND,
  INTERACT,
  REST,
  JOURNAL
};

} // namespace model