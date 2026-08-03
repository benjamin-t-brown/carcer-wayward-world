#include "game/map/TileTriggers.h"
#include "bmin/StringInterop.h"
#include "game/map/MapWalkability.h"
#include "sdl2w/L10n.h"

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

  auto partyIndex = player.currentPartyMemberIndex;
  if (partyIndex < 0 || static_cast<size_t>(partyIndex) >= player.party.size()) {
    partyIndex = 0;
  }
  const auto& member = player.party[static_cast<size_t>(partyIndex)];

  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    if (activeMap.characters[i].id == member.instanceId) {
      return &activeMap.characters[i];
    }
  }
  return nullptr;
}

model::CharacterInstance*
placePartyAvatarAt(model::ActiveMap& activeMap, model::Player& player, int x, int y) {
  if (player.party.empty()) {
    return nullptr;
  }

  auto partyIndex = player.currentPartyMemberIndex;
  if (partyIndex < 0 || static_cast<size_t>(partyIndex) >= player.party.size()) {
    partyIndex = 0;
  }
  const auto& member = player.party[static_cast<size_t>(partyIndex)];

  auto* avatar = findPartyAvatarOnActiveMap(activeMap, player);
  if (avatar) {
    avatar->x = x;
    avatar->y = y;
    return avatar;
  }

  auto instance = model::CharacterInstance{};
  instance.id = member.instanceId;
  instance.name = member.name.empty() ? member.params.name : member.name;
  instance.templateName =
      member.templateName.empty() ? member.params.name : member.templateName;
  instance.x = x;
  instance.y = y;
  instance.spawnX = x;
  instance.spawnY = y;
  activeMap.characters.pushBack(std::move(instance));
  return findPartyAvatarOnActiveMap(activeMap, player);
}

void queueStepTriggersAt(state::Triggers& triggers,
                         const model::MapInstance& map,
                         int x,
                         int y) {
  triggers.pendingSpecialEventId.reset();
  triggers.pendingTravel.reset();

  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return;
  }

  if (tile->eventTrigger && !tile->eventTrigger->requiresLook) {
    triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
    return;
  }

  if (tile->travelTrigger && !tile->travelTrigger->requiresAction) {
    triggers.pendingTravel = *tile->travelTrigger;
  }
}

void queueActionTravelAtStanding(state::Triggers& triggers,
                                 const model::MapInstance& map,
                                 int x,
                                 int y) {
  triggers.pendingTravel.reset();

  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile || !tile->travelTrigger || !tile->travelTrigger->requiresAction) {
    return;
  }

  triggers.pendingTravel = *tile->travelTrigger;
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

  for (const auto& item : activeMap.items) {
    if (item.x != worldX || item.y != worldY) {
      continue;
    }
    appendLine(itemLabelAt(database, item.itemTemplateName));
  }

  return message;
}

} // namespace game
