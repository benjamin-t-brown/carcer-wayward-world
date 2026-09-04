module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <stdexcept>
#include <charconv>
#include <system_error>

export module carcer.model.templates:CharacterTemplate;
export import bmin.containers;
import bmin.string_interop;
export import :CharacterStats;
import sdl2w;
#include "macros.h"

export {

// --- from model/templates/CharacterTemplate.h ---
namespace model {

enum class CharacterTemplateType {
  TOWNSPERSON,
  TOWNSPERSON_STATIC,
  ENEMY,
  ENEMY_STATIC,
};

enum class CharacterTemplateBehaviorName {
  MOVE_RANDOMLY,
  IMMOBILE,
  IMMOBILE_UNTIL_ENEMY_SPOTTED,
  SEEK_MARKER,
  MOVE_LEFT_RIGHT,
  MOVE_UP_DOWN,
};

enum class CombatBehaviorName {
  SEEK_AND_MELEE,
};

struct CharacterTemplateTalk {
  bmin::String talkName;
  bmin::String portraitName;
};

struct CharacterTemplateBehavior {
  bmin::String behaviorName;
};

struct CharacterTemplateCombat {
  int hp = 0;
  int mp = 0;
  bmin::String dropTable;
};

struct CharacterTemplateCombatBehavior {
  CombatBehaviorName town = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combat = CombatBehaviorName::SEEK_AND_MELEE;
};

struct CharacterTemplateSound {
  bmin::String deathSoundName;
  bmin::String weaponSoundName;
};

struct CharacterTemplateStatus {
  bmin::String status;
};

struct CharacterTemplateVision {
  int radius = 0;
};

struct CharacterTemplate {
  CharacterTemplateType type;
  bmin::String name;
  bmin::String label;
  bmin::String spritesheetName;
  bmin::String spriteOffset;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterStats stats;
  CharacterTemplateCombat combat;
  CharacterTemplateCombatBehavior combatBehavior;
  CharacterTemplateSound sound;
  bmin::DynArray<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
  /** SpellTemplate.name values applied when constructing a CharacterPlayer. */
  bmin::DynArray<bmin::String> startingKnownSpells;
  /** Optional prepared subset; only names also in known after apply are kept. */
  bmin::DynArray<bmin::String> startingReadySpells;
};

bmin::String characterGetSprite(const CharacterTemplate& character);
bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                             int indexOffset);

/** Initialize CharacterStats from a template's stats block. */
void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl);

} // namespace model

} // export

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

void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl) {
  out = tmpl.stats;
}

} // namespace model
