#include "LayerSpellAllyTarget.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "actions/navigation/UiSelectSpellAllyTarget.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/helpers/uiSounds.h"
#include "ui/popups/PopupSpellAllyTarget.h"

namespace layers {

LayerSpellAllyTarget::LayerSpellAllyTarget(sdl2w::Window* _window,
                                           const bmin::String& casterId,
                                           const bmin::String& spellId)
    : UiLayer(_window, LAYER_ID), casterId(casterId), spellId(spellId) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto& state = stateManager->getState();

  auto [windowWidth, windowHeight] = window->getDims();

  auto popup = new ui::PopupSpellAllyTarget(window);
  popup->setId("popupSpellAllyTarget");

  ui::PopupSpellAllyTargetProps popupProps;
  popupProps.casterId = casterId;
  popupProps.spellId = spellId;
  popupProps.titleText = spellId;
  auto* database = getDatabase();
  if (database != nullptr) {
    const auto* spell = database->findSpellTemplate(bmin::toStringView(spellId));
    if (spell != nullptr) {
      popupProps.titleText = spell->label;
      if (popupProps.titleText.empty()) {
        const auto* ability =
            database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
        if (ability != nullptr) {
          popupProps.titleText = ability->label;
        }
      }
      if (popupProps.titleText.empty()) {
        popupProps.titleText = spell->name;
      }
    }
  }

  for (const auto& member : state.player.party) {
    ui::PopupSpellAllyTargetAlly entry;
    entry.id = member.instanceId;
    entry.label = member.params.label.empty() ? member.name : member.params.label;
    if (entry.label.empty()) {
      entry.label = member.instanceId;
    }
    entry.iconSprite = model::characterPlayerGetSprite(member);
    popupProps.allies.pushBack(std::move(entry));
  }
  if (popupProps.allies.empty()) {
    popupProps.statusText = TRANSLATE("No party members.");
  } else {
    popupProps.statusText = TRANSLATE("Choose a party member.");
  }

  popup->setProps(popupProps);
  popup->setScale(1.f);
  auto [popupW, popupH] = popup->getDims();
  popup->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popup->build();

  addUiElement(bmin::UniquePtr<ui::UiElement>(popup));

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));
}

void LayerSpellAllyTarget::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  if (const auto allyIndex = ui::getAlphabeticShortcutIndexFromKey(key)) {
    auto* popup = getUiElement<ui::PopupSpellAllyTarget>("popupSpellAllyTarget");
    if (popup && *allyIndex < static_cast<int>(popup->getProps().allies.size())) {
      const auto& ally = popup->getProps().allies[*allyIndex];
      if (!ally.id.empty()) {
        ui::playButtonSound(window);
        stateManager->enqueueAction(
            state::makeAction<state::actions::UiSelectSpellAllyTarget>(
                spellId, casterId, ally.id),
            0);
      }
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  ui::playButtonSound(window);
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiRemoveLayer>(
          bmin::String(LAYER_ID.data(), LAYER_ID.size())),
      0);
}

} // namespace layers
