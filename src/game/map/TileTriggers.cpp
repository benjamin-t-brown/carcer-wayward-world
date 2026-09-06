module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

module carcer.game.map;
import bmin.containers;
import carcer.db;
import carcer.model;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {
namespace {

bmin::String itemLabelAt(const db::Database& database,
                         const bmin::String& itemTemplateName) {
  try {
    const auto& itemTemplate =
        database.getItemTemplate(bmin::toStringView(itemTemplateName));
    if (!itemTemplate.label.empty()) {
      return itemTemplate.label;
    }
    return itemTemplate.name;
  } catch (...) {
    return itemTemplateName;
  }
}

bmin::String characterLabelAt(const db::Database& database,
                              const model::CharacterInstance& character) {
  try {
    const auto& characterTemplate =
        database.getCharacterTemplate(bmin::toStringView(character.templateName));
    if (!characterTemplate.label.empty()) {
      return characterTemplate.label;
    }
    if (!characterTemplate.name.empty()) {
      return characterTemplate.name;
    }
  } catch (...) {
  }
  if (!character.name.empty()) {
    return character.name;
  }
  return character.templateName;
}

} // namespace

model::CharacterInstance* findPartyAvatarOnActiveMap(model::ActiveMap& activeMap,
                                                     model::Player& player) {
  return const_cast<model::CharacterInstance*>(
      findPartyAvatarOnActiveMap(static_cast<const model::ActiveMap&>(activeMap),
                                 static_cast<const model::Player&>(player)));
}

const model::CharacterInstance*
findPartyAvatarOnActiveMap(const model::ActiveMap& activeMap,
                           const model::Player& player) {
  if (player.party.empty()) {
    return nullptr;
  }

  // Town/outdoor movement always uses the party leader avatar (party[0]).
  // UI selection (selectedPartyMemberId) must not affect which avatar moves.
  const auto& leader = player.party[0];
  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    if (activeMap.characters[i].id == leader.instanceId) {
      return &activeMap.characters[i];
    }
  }
  return nullptr;
}

model::CharacterInstance* placePartyAvatarAt(model::ActiveMap& activeMap,
                                             model::Player& player,
                                             int x,
                                             int y,
                                             const db::Database* database) {
  if (player.party.empty()) {
    return nullptr;
  }

  const auto& leader = player.party[0];

  auto* avatar = findPartyAvatarOnActiveMap(activeMap, player);
  if (avatar) {
    avatar->x = x;
    avatar->y = y;
    return avatar;
  }

  auto instance = model::CharacterInstance{};
  instance.id = leader.instanceId;
  instance.name = leader.name.empty() ? leader.params.name : leader.name;
  instance.templateName =
      leader.templateName.empty() ? leader.params.name : leader.templateName;
  instance.x = x;
  instance.y = y;
  instance.spawnX = x;
  instance.spawnY = y;
  if (database) {
    applyCharacterTemplateFromDatabase(instance, *database);
  }
  activeMap.characters.pushBack(std::move(instance));
  return findPartyAvatarOnActiveMap(activeMap, player);
}

const model::CharacterInstance*
findDropCharacterOnActiveMap(const model::ActiveMap& activeMap,
                             const model::Player& player,
                             const bmin::String& characterId) {
  if (!characterId.empty()) {
    for (size_t i = 0; i < activeMap.characters.size(); i++) {
      if (activeMap.characters[i].id == characterId) {
        return &activeMap.characters[i];
      }
    }
  }
  return findPartyAvatarOnActiveMap(activeMap, player);
}

model::CharacterInstance* findDropCharacterOnActiveMap(model::ActiveMap& activeMap,
                                                       model::Player& player,
                                                       const bmin::String& characterId) {
  return const_cast<model::CharacterInstance*>(findDropCharacterOnActiveMap(
      static_cast<const model::ActiveMap&>(activeMap),
      static_cast<const model::Player&>(player),
      characterId));
}

StepTriggerResult resolveStepTriggersAt(const model::MapInstance& map,
                                        int x,
                                        int y) {
  auto result = StepTriggerResult{};
  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return result;
  }

  if (tile->eventTrigger && !tile->eventTrigger->requiresLook) {
    result.specialEventId = tile->eventTrigger->eventId;
    return result;
  }

  if (tile->travelTrigger && !tile->travelTrigger->requiresAction) {
    result.travel = *tile->travelTrigger;
  }
  return result;
}

std::optional<model::TravelTrigger>
resolveActionTravelAtStanding(const model::MapInstance& map, int x, int y) {
  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile || !tile->travelTrigger || !tile->travelTrigger->requiresAction) {
    return std::nullopt;
  }

  return *tile->travelTrigger;
}

bmin::String formatExamineMessage(const model::MapInstance& map,
                                  const model::ActiveMap& activeMap,
                                  int worldX,
                                  int worldY,
                                  int localX,
                                  int localY,
                                  const db::Database& database) {
  bmin::String message = TRANSLATE("Examine:");

  auto appendLine = [&](const bmin::String& line) {
    if (line.empty()) {
      return;
    }
    message += "\n";
    message += line;
  };

  const auto* tile = tileAtCurrentLayer(map, localX, localY);
  if (tile) {
    if (const auto* meta = resolveTileMetadata(*tile, database)) {
      appendLine(meta->description);
    }
  }

  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    const auto& character = activeMap.characters[i];
    if (character.x != worldX || character.y != worldY) {
      continue;
    }
    appendLine(characterLabelAt(database, character));
  }

  const bool tileIsContainer =
      tile != nullptr && isTileEffectivelyContainer(*tile, database);
  if (!tileIsContainer) {
    for (const auto& item : activeMap.items) {
      if (item.x != worldX || item.y != worldY) {
        continue;
      }
      appendLine(itemLabelAt(database, item.itemTemplateName));
    }
  }

  return message;
}

void resolveWorldActionMode(model::World& world,
                            const model::Player& player,
                            model::WorldActionMode mode,
                            const bmin::String& spellId,
                            const bmin::String& chId) {
  world.actionMode = mode;
  if (mode == model::WorldActionMode::SPELL) {
    if (!spellId.empty()) {
      world.pendingSpellId = spellId;
    }
    if (!chId.empty()) {
      world.pendingChId = chId;
    }
  } else {
    world.pendingSpellId = bmin::String{};
  }
  if (mode == model::WorldActionMode::NONE) {
    world.actionAimTile.reset();
    return;
  }

  // Combat SPELL: aim under the active combat character (caster).
  if (mode == model::WorldActionMode::SPELL && world.combat.active &&
      !world.combat.activeCharacterId.empty()) {
    for (const auto& character : world.activeMap.characters) {
      if (character.id == world.combat.activeCharacterId) {
        world.actionAimTile = model::TileXY{character.x, character.y};
        return;
      }
    }
  }

  // EXAMINE / TALK / SPELL fallback: start aim under the party leader avatar.
  const auto* avatar = findPartyAvatarOnActiveMap(world.activeMap, player);
  if (avatar) {
    world.actionAimTile = model::TileXY{avatar->x, avatar->y};
  } else {
    world.actionAimTile.reset();
  }
}

} // namespace game
