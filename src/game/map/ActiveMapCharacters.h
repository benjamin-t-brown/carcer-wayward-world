#pragma once

#include "model/instances/World.hpp"

namespace game {

// Scans ActiveMap::characters in world coordinates.
inline model::CharacterInstance*
findCharacterById(model::ActiveMap& activeMap, const bmin::String& characterId) {
  for (auto it = activeMap.characters.begin(); it != activeMap.characters.end(); ++it) {
    if (it->id == characterId) {
      return it;
    }
  }
  return nullptr;
}

inline model::CharacterInstance* findCharacterAt(model::ActiveMap& activeMap,
                                                 int worldX,
                                                 int worldY,
                                                 const bmin::String& excludeId) {
  for (auto it = activeMap.characters.begin(); it != activeMap.characters.end(); ++it) {
    if (it->x == worldX && it->y == worldY && it->id != excludeId) {
      return it;
    }
  }
  return nullptr;
}

inline model::CharacterInstance*
findCharacterAt(model::ActiveMap& activeMap, int worldX, int worldY) {
  return findCharacterAt(activeMap, worldX, worldY, bmin::String{});
}

inline bmin::DynArray<model::CharacterInstance*>
findAllCharactersAt(model::ActiveMap& activeMap, int worldX, int worldY) {
  bmin::DynArray<model::CharacterInstance*> characters;
  for (auto it = activeMap.characters.begin(); it != activeMap.characters.end(); ++it) {
    if (it->x == worldX && it->y == worldY) {
      characters.pushBack(it);
    }
  }
  return characters;
}

} // namespace game
