module;
#include <cstddef>

module carcer.actions;
import sdl2w;
import bmin.string_interop;
import carcer.model.templates;
#include "macros.h"

namespace state {

namespace actions {

void UiSelectSpellCast::act() {
  if (!state) {
    return;
  }
  auto* database = getDatabase();
  if (!database) {
    LOG(WARN) << "UiSelectSpellCast::act: database is nullptr" << LOG_ENDL;
    return;
  }

  auto* character = model::playerFindPartyMemberById(
      state->player, state->world.combat.activeCharacterId);
  if (!character) {
    LOG(WARN) << "UiSelectSpellCast::act: active combat party member not found"
              << LOG_ENDL;
    pushWarning(TRANSLATE("Cannot cast that spell."));
    return;
  }

  const auto* spell = database->findSpellTemplate(bmin::toStringView(spellId));
  if (spell == nullptr) {
    pushWarning(TRANSLATE("Cannot cast that spell."));
    return;
  }

  // const auto manaCost = model::spellAbilityManaCost(*spell, *database);
  // if (character->currentMp < manaCost) {
  //   pushWarning(TRANSLATE("Not enough mana."));
  //   return;
  // }

  // if (!model::characterHasSpellReady(
  //         *character, bmin::toStringView(spellId), *database)) {
  //   pushWarning(TRANSLATE("Required runes are not equipped."));
  //   return;
  // }

  // const auto* ability =
  //     database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
  // if (ability == nullptr ||
  //     ability->targetSelect.targetType != model::TargetSelectType::TARGET_ZONE) {
  //   pushWarning(TRANSLATE("That spell cannot be aimed on the map."));
  //   return;
  // }

  UiRemoveLayer(LayerId::SpellCast).execute(state);
  WorldSetActionMode(model::WorldActionMode::SPELL, {.spellId = spellId, .chId = chId})
      .execute(state);
}

} // namespace actions

} // namespace state
