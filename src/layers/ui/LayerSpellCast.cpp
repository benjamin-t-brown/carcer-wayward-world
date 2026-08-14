#include "LayerSpellCast.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "game/combat/SpellRules.h"
#include "lib/StringUtil.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/RuneTypes.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/minipages/MinipageSpellCast.h"

namespace layers {

LayerSpellCast::LayerSpellCast(sdl2w::Window* _window, const bmin::String& chId)
    : Layer(_window, LAYER_ID), chId(chId) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  auto& state = stateManager->getState();

  auto minipageSpellCast = new ui::MinipageSpellCast(window);
  minipageSpellCast->setId("minipageSpellCast");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  minipageSpellCast->setPos(0, 0);
  minipageSpellCast->setScale(scale);
  auto minipageInitProps = minipageSpellCast->getProps();
  minipageInitProps.casterId = chId;
  minipageInitProps.width = static_cast<int>(windowWidth / scale);
  minipageInitProps.height = static_cast<int>(windowHeight / scale);
  minipageInitProps.doneButtonRemoveLayerId = strutil::fromStringView(LAYER_ID);
  minipageInitProps.titleText = TRANSLATE("Cast Spell");
  auto chPlayer = model::playerFindPartyMemberById(state.player, chId);

  if (chPlayer == nullptr) {
    LOG(ERROR) << "LayerSpellCast::LayerSpellCast: party member not found: " << chId
               << LOG_ENDL;
    remove();
    return;
  }

  for (const auto& spellName : chPlayer->knownSpells) {
    minipageInitProps.spells.pushBack(makeSpellEntry(*database, spellName));
  }
  if (minipageInitProps.spells.empty()) {
    minipageInitProps.statusText = TRANSLATE("No known spells.");
  } else {
    minipageInitProps.statusText.clear();
  }

  minipageSpellCast->setProps(minipageInitProps);

  addUiElement(minipageSpellCast);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

ui::MinipageSpellCastSpell LayerSpellCast::makeSpellEntry(const db::Database& database,
                                                          const bmin::String& spellName) {
  ui::MinipageSpellCastSpell entry;
  entry.id = spellName;

  const auto* spell = database.findSpellTemplate(bmin::toStringView(spellName));
  if (spell == nullptr) {
    LOG(ERROR) << "LayerSpellCast::makeSpellEntry: spell template not found: "
               << spellName << LOG_ENDL;
    entry.label = spellName;
    return entry;
  }

  const auto* ability =
      database.findAbilityTemplate(bmin::toStringView(spell->abilityName));
  entry.label = spell->label;
  if (entry.label.empty() && ability != nullptr) {
    entry.label = ability->label;
  }
  entry.iconSprite = spell->icon;
  if (entry.iconSprite.empty() && ability != nullptr) {
    entry.iconSprite = ability->icon;
  }
  for (const auto& req : spell->requiredRunes) {
    const auto sprite = model::runeTypeToSpriteName(req.type);
    const int count = req.count > 0 ? req.count : 1;
    for (int i = 0; i < count; ++i) {
      entry.requiredRuneSprites.pushBack(sprite);
    }
  }
  return entry;
}

void LayerSpellCast::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  if (!ui::isCancelActionKey(key)) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(bmin::String(LAYER_ID.data(), LAYER_ID.size())),
      0);
}

} // namespace layers
