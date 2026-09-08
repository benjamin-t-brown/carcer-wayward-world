#include "LayerEquipRunes.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "actions/navigation/UiAdjustEquippedRune.hpp"
#include "actions/navigation/UiCancelEquipRunes.hpp"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/minipages/MinipageEquipRunes.h"

namespace layers {

LayerEquipRunes::LayerEquipRunes(sdl2w::Window* _window,
                                 const bmin::String& _characterPlayerId)
    : UiLayer(_window, LAYER_ID), characterPlayerId(_characterPlayerId) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto* characterPlayer =
      model::playerFindPartyMemberById(getStateManager()->getState().player,
                                       characterPlayerId);
  if (characterPlayer == nullptr) {
    remove();
    return;
  }
  equippedSnapshot = characterPlayer->equippedRunes;

  auto minipage = new ui::MinipageEquipRunes(window);
  minipage->setId("minipageEquipRunes");
  addUiElement(bmin::UniquePtr<ui::UiElement>(minipage));

  syncFromCharacter();

  subscribeAction<state::ActionEvent::UiAdjustEquippedRune>(
      [this](auto&, auto&) { syncFromCharacter(); });
}

void LayerEquipRunes::restoreSnapshot() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto* characterPlayer =
      model::playerFindPartyMemberById(stateManager->getState().player,
                                       characterPlayerId);
  if (characterPlayer == nullptr) {
    return;
  }
  characterPlayer->equippedRunes = equippedSnapshot;
}

void LayerEquipRunes::syncFromCharacter() {
  auto* minipage = getUiElement<ui::MinipageEquipRunes>("minipageEquipRunes");
  if (minipage == nullptr || !hasStateManager()) {
    return;
  }

  auto* characterPlayer =
      model::playerFindPartyMemberById(getStateManager()->getState().player,
                                       characterPlayerId);
  if (characterPlayer == nullptr) {
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();
  ui::MinipageEquipRunesProps props;
  props.width = windowWidth;
  props.height = windowHeight;
  props.characterPlayerId = characterPlayerId;
  props.characterPlayerLabel = characterPlayer->params.label;

  props.runeSlots.clear();
  for (size_t i = 0; i < model::CharacterPlayer::kRuneSlotCount; ++i) {
    if (i < characterPlayer->equippedRunes.size()) {
      props.runeSlots.pushBack(ui::MinipageEquipRunesSlot{
          .filled = true,
          .iconSprite =
              model::runeTypeToSpriteName(characterPlayer->equippedRunes[i]),
      });
    } else {
      props.runeSlots.pushBack(ui::MinipageEquipRunesSlot{.filled = false});
    }
  }

  props.runeRows.clear();
  for (int i = 0; i < model::kRuneTypeCount; ++i) {
    const auto runeType = model::runeTypeFromIndex(i);
    props.runeRows.pushBack(ui::MinipageEquipRunesRow{
        .type = runeType,
        .iconSprite = model::runeTypeToSpriteName(runeType),
        .availableCount = model::characterPlayerCountAvailableRunesOfType(
            *characterPlayer, runeType),
        .equippedCount = model::characterPlayerCountEquippedRunesOfType(
            *characterPlayer, runeType),
    });
  }

  minipage->setPos(0, 0);
  minipage->setScale(1.f);
  minipage->setProps(props);
}

void LayerEquipRunes::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto* stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(state::makeAction<state::actions::UiCancelEquipRunes>(),
                              0);
}

} // namespace layers
