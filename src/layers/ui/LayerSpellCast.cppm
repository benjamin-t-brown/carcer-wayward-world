module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string>
#include <typeinfo>
#include <typeindex>

export module carcer.layers.LayerSpellCast;
export import carcer.layers.Layer;
export import carcer.db;
export import carcer.ui.minipages;
import sdl2w;
import carcer.actions.ui.UiRemoveLayer;
import carcer.lib.StringUtil;
import carcer.model.instances.Player;
import carcer.model.templates;
import carcer.ui.components;
import carcer.ui.helpers;
import bmin.containers;
import bmin.string_interop;
#include "macros.h"

export {

namespace layers {

/** Combat spell-cast list for the active party member's known spells. */
class LayerSpellCast : public Layer {
  bmin::String chId;
  static ui::MinipageSpellCastSpell makeSpellEntry(const db::Database& database,
                                                   const bmin::String& spellName);

public:
  explicit LayerSpellCast(sdl2w::Window* _window, const bmin::String& chId);
  ~LayerSpellCast() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers

} // export

namespace layers {

LayerSpellCast::LayerSpellCast(sdl2w::Window* _window, const bmin::String& chId)
    : Layer(_window, state::LayerId::SpellCast), chId(chId) {
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
  minipageInitProps.doneButtonRemoveLayerId = strutil::fromStringView(state::layerIdString(state::LayerId::SpellCast));
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
      entry.requiredRuneSprites.push_back(sprite);
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
      new state::actions::UiRemoveLayer(state::LayerId::SpellCast),
      0);
}

} // namespace layers
