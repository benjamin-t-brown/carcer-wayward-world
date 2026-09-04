module;
#include <utility>

export module carcer.actions.ui.UiSelectSpellCast;
export import carcer.state;
import sdl2w;
import bmin.string_interop;
import carcer.actions.ui.UiRemoveLayer;
import carcer.actions.world.WorldSetActionMode;
import carcer.game.combat;
#include "macros.h"

export {

namespace state {

namespace actions {

/** Validate a known spell for combat cast; on success close cast list and enter SPELL
 * aim. */
class UiSelectSpellCast : public AbstractAction {
  bmin::String spellId;
  bmin::String chId;

  void pushWarning(bmin::String message) {
    UiFloatingNotification notification;
    notification.id = model::createRandomId();
    notification.message = std::move(message);
    notification.type = UiFloatingNotificationType::WARNING;
    model::timerStructStart(notification.timer,
                            state->settings.floatingNotificationDurationMs);
    state->uiState.floatingNotifications.pushBack(std::move(notification));
  }

  void act() override {
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

public:
  explicit UiSelectSpellCast(const bmin::String& _spellId, const bmin::String& _chId)
      : spellId((_spellId)), chId((_chId)) {}
};

} // namespace actions

} // namespace state

} // export
