#include "model/templates/CharacterTemplate.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "bmin/StringInterop.h"
#include <charconv>
#include <stdexcept>
#include <system_error>

namespace model {

namespace {

int parseSpriteOffsetIndex(const bmin::String& spriteOffset) {
  const auto view = bmin::toStringView(spriteOffset);
  int value = 0;
  const auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value);
  if (ec != std::errc{} || ptr != view.data() + view.size()) {
    throw std::runtime_error("Invalid character spriteOffset");
  }
  return value;
}

bmin::String buildCharacterSpriteName(const bmin::String& spritesheetName,
                                      const bmin::String& spriteOffset,
                                      int indexOffset) {
  const auto index = parseSpriteOffsetIndex(spriteOffset) + indexOffset;
  return spritesheetName + "_" + bmin::toString(index);
}

} // namespace

bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                               int indexOffset) {
  return buildCharacterSpriteName(
      characterTemplate.spritesheetName, characterTemplate.spriteOffset, indexOffset);
}

bmin::String characterGetSprite(const CharacterTemplate& characterTemplate) {
  return characterGetSpriteAtIndexOffset(characterTemplate, 0);
}

void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate) {
  character.type = characterTemplate.type;
  character.label = characterTemplate.label;
  character.behaviorName = characterTemplate.behavior.behaviorName;
  character.visionRadius = characterTemplate.vision.radius;
  character.combatBehaviorTown = characterTemplate.combatBehavior.town;
  character.combatBehaviorCombat = characterTemplate.combatBehavior.combat;
  character.maxHp = characterTemplate.combat.hp;
  if (character.name.empty()) {
    character.name = characterTemplate.label.empty() ? characterTemplate.name
                                                     : characterTemplate.label;
  }
  if (character.templateName.empty()) {
    character.templateName = characterTemplate.name;
  }
}

bool tryApplyCharacterTemplateToInstance(CharacterInstance& character,
                                         const db::Database& database) {
  if (character.templateName.empty()) {
    return false;
  }
  try {
    const auto& characterTemplate =
        database.getCharacterTemplate(bmin::toStringView(character.templateName));
    applyCharacterTemplateToInstance(character, characterTemplate);
    return true;
  } catch (...) {
    return false;
  }
}

void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate) {
  character.knownSpells.clear();
  for (const auto& spellName : characterTemplate.startingKnownSpells) {
    if (spellName.empty()) {
      continue;
    }
    bool alreadyKnown = false;
    for (const auto& known : character.knownSpells) {
      if (known == spellName) {
        alreadyKnown = true;
        break;
      }
    }
    if (!alreadyKnown) {
      character.knownSpells.pushBack(spellName);
    }
  }

  character.readySpells.clear();
  for (const auto& spellName : characterTemplate.startingReadySpells) {
    if (spellName.empty()) {
      continue;
    }
    bool isKnown = false;
    for (const auto& known : character.knownSpells) {
      if (known == spellName) {
        isKnown = true;
        break;
      }
    }
    if (!isKnown) {
      continue;
    }
    bool alreadyReady = false;
    for (const auto& ready : character.readySpells) {
      if (ready == spellName) {
        alreadyReady = true;
        break;
      }
    }
    if (!alreadyReady) {
      character.readySpells.pushBack(spellName);
    }
  }
}

} // namespace model
