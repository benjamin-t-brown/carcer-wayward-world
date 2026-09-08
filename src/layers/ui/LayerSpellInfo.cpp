#include "LayerSpellInfo.h"
#include "bmin/StringInterop.h"
#include "game/combat/SpellRules.h"
#include "model/templates/RuneTypes.h"
#include "sdl2w/Logger.h"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/popups/PopupSpellInfo.h"

namespace layers {

LayerSpellInfo::LayerSpellInfo(sdl2w::Window* _window, const bmin::String& spellName)
    : UiLayer(_window, LAYER_ID) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (spellName.empty()) {
    LOG(ERROR) << "LayerSpellInfo: spellName is empty" << LOG_ENDL;
    remove();
    return;
  }

  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerSpellInfo: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }

  const auto* spell = database->findSpellTemplate(bmin::toStringView(spellName));
  if (spell == nullptr) {
    LOG(ERROR) << "LayerSpellInfo: spell template not found: " << spellName << LOG_ENDL;
    remove();
    return;
  }

  const auto* ability =
      database->findAbilityTemplate(bmin::toStringView(spell->abilityName));

  ui::PopupSpellInfoProps popupProps;
  popupProps.label = spell->label;
  if (popupProps.label.empty() && ability != nullptr) {
    popupProps.label = ability->label;
  }
  if (popupProps.label.empty()) {
    popupProps.label = spell->name;
  }
  popupProps.description = spell->description;
  if (popupProps.description.empty() && ability != nullptr) {
    popupProps.description = ability->description;
  }
  popupProps.spriteName = spell->icon;
  if (popupProps.spriteName.empty() && ability != nullptr) {
    popupProps.spriteName = ability->icon;
  }
  popupProps.manaCost = model::spellAbilityManaCost(*spell, *database);
  for (const auto& req : spell->requiredRunes) {
    popupProps.requiredRunes.pushBack(ui::PopupSpellInfoRuneReq{
        .iconSprite = model::runeTypeToSpriteName(req.type),
        .count = req.count,
    });
  }

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;
  popupProps.orientation = orientation;

  auto popup = new ui::PopupSpellInfo(window, nullptr, orientation);
  popup->setId("popupSpellInfo");
  popup->setProps(popupProps);

  auto [popupW, popupH] = popup->getDims();
  popup->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popup->setScale(1.0f);
  popup->build();

  addUiElement(popup);
}

void LayerSpellInfo::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(state::makeAction<state::actions::UiRemoveLayer>(bmin::String(LAYER_ID.data(), LAYER_ID.size())),
      0);
}

} // namespace layers
