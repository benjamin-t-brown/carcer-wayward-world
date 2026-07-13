#include "model/TileTriggers.h"
#include "model/MapWalkability.h"
#include "bmin/StringInterop.h"

namespace model {
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
                              const CharacterInstance& character) {
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

CharacterInstance* findPartyAvatarOnMap(MapInstance& map, Player& player) {
  return const_cast<CharacterInstance*>(findPartyAvatarOnMap(
      static_cast<const MapInstance&>(map), static_cast<const Player&>(player)));
}

const CharacterInstance* findPartyAvatarOnMap(const MapInstance& map,
                                              const Player& player) {
  if (player.party.empty()) {
    return nullptr;
  }

  auto partyIndex = player.currentPartyMemberIndex;
  if (partyIndex < 0 || static_cast<size_t>(partyIndex) >= player.party.size()) {
    partyIndex = 0;
  }
  const auto& member = player.party[static_cast<size_t>(partyIndex)];

  for (size_t i = 0; i < map.characters.size(); i++) {
    if (map.characters[i].id == member.instanceId) {
      return &map.characters[i];
    }
  }
  return nullptr;
}

CharacterInstance* placePartyAvatarAt(MapInstance& map, Player& player, int x, int y) {
  if (player.party.empty()) {
    return nullptr;
  }

  auto partyIndex = player.currentPartyMemberIndex;
  if (partyIndex < 0 || static_cast<size_t>(partyIndex) >= player.party.size()) {
    partyIndex = 0;
  }
  const auto& member = player.party[static_cast<size_t>(partyIndex)];

  auto* avatar = findPartyAvatarOnMap(map, player);
  if (avatar) {
    avatar->x = x;
    avatar->y = y;
    return avatar;
  }

  auto instance = CharacterInstance{};
  instance.id = member.instanceId;
  instance.name = member.name.empty() ? member.params.name : member.name;
  instance.templateName =
      member.templateName.empty() ? member.params.name : member.templateName;
  instance.x = x;
  instance.y = y;
  map.characters.pushBack(std::move(instance));
  return findPartyAvatarOnMap(map, player);
}

void queueStepTriggersAt(World& world, const MapInstance& map, int x, int y) {
  world.pendingSpecialEventId.reset();
  world.pendingTravel.reset();

  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return;
  }

  if (tile->eventTrigger && !tile->eventTrigger->isLookTrigger) {
    world.pendingSpecialEventId = tile->eventTrigger->eventId;
    return;
  }

  if (tile->travelTrigger) {
    world.pendingTravel = *tile->travelTrigger;
  }
}

bmin::String formatExamineMessage(const MapInstance& map,
                                  int x,
                                  int y,
                                  const db::Database& database) {
  bmin::String message = "Examine:";

  auto appendLine = [&](const bmin::String& line) {
    if (line.empty()) {
      return;
    }
    message += "\n";
    message += line;
  };

  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (tile) {
    if (const auto* meta = resolveTileMetadata(*tile, database)) {
      appendLine(meta->description);
    }
  }

  for (size_t i = 0; i < map.characters.size(); i++) {
    const auto& character = map.characters[i];
    if (character.x != x || character.y != y) {
      continue;
    }
    appendLine(characterLabelAt(database, character));
  }

  for (const auto& item : map.items) {
    if (item.x != x || item.y != y) {
      continue;
    }
    appendLine(itemLabelAt(database, item.itemTemplateName));
  }

  return message;
}

} // namespace model
