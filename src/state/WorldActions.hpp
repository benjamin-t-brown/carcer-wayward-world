#pragma once

#include <array>

namespace state {

// World action types enum
enum class WorldActionType {
  NONE,
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

struct WorldActionUiState {
  const std::array<WorldActionType, 13> townModeActionTypes = {
      WorldActionType::START_FIGHT,
      WorldActionType::EXAMINE,
      WorldActionType::TALK,
      WorldActionType::ABILITY,
      WorldActionType::SNEAK,
      WorldActionType::JUMP,
      WorldActionType::SHOOT,
      WorldActionType::INTERACT,
      WorldActionType::GET,
      WorldActionType::JOURNAL,
      WorldActionType::INVENTORY,
      WorldActionType::STATUS,
      WorldActionType::MAP,
  };
  const std::array<WorldActionType, 9> townModeFightActionTypes = {
      WorldActionType::END_FIGHT,
      WorldActionType::EXAMINE,
      WorldActionType::ABILITY,
      WorldActionType::JUMP,
      WorldActionType::SHOOT,
      WorldActionType::DEFEND,
      WorldActionType::GET,
      WorldActionType::INVENTORY,
      WorldActionType::STATUS,
  };
  const std::array<WorldActionType, 7> outdoorModeActionTypes = {
      WorldActionType::EXAMINE,
      WorldActionType::ABILITY,
      WorldActionType::MAP_OUTDOOR,
      WorldActionType::REST,
      WorldActionType::INVENTORY,
      WorldActionType::STATUS,
      WorldActionType::JOURNAL,
  };
};

} // namespace state
