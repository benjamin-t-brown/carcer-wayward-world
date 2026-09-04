module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <algorithm>
#include <optional>

export module carcer.layers.screens;
export import carcer.layers;
export import carcer.model.templates;
export import carcer.db;
export import carcer.ui.pages.PageMagicSetup;
export import carcer.ui.elements;
export import carcer.model.instances;
export import carcer.ui.minipages;
import sdl2w;
import carcer.model.instances;
import carcer.ui.components;
import carcer.ui.popups;
import bmin.containers;
import bmin.string_interop;
import carcer.actions;
import carcer.ui.helpers;
import carcer.ui.minipages;
import carcer.ui.pages.PageInventory;
import carcer.game.combat;
import carcer.game.map;
import carcer.lib.StringUtil;
import carcer.ui.ObserverRemoveLayer;
import carcer.ui.core;
import carcer.ui.elements;
import carcer.model.templates;
#include "macros.h"

export {

namespace layers {

class LayerDropConfirm : public Layer {
public:
  explicit LayerDropConfirm(sdl2w::Window* _window,
                            bmin::String characterPlayerId,
                            bmin::String itemId);
  ~LayerDropConfirm() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

namespace layers {

class LayerEquipRunes : public Layer {
private:
  bmin::String characterPlayerId;
  bmin::DynArray<model::RuneType> equippedSnapshot;

  void syncFromCharacter();

public:
  explicit LayerEquipRunes(sdl2w::Window* _window, const bmin::String& characterPlayerId);
  ~LayerEquipRunes() override = default;

  void restoreSnapshot();
  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers

namespace layers {

class LayerGiveContext : public Layer {
public:
  explicit LayerGiveContext(sdl2w::Window* _window,
                            bmin::String fromCharacterPlayerId,
                            bmin::String itemId);
  ~LayerGiveContext() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

namespace layers {

class LayerInventory : public Layer {
public:
  explicit LayerInventory(sdl2w::Window* _window);
  virtual ~LayerInventory() = default;

  void onKeyDown(std::string_view key, int keyCode) override;

  void syncInventoryPartyMember();
};

} // namespace layers

namespace layers {

class LayerInventoryContext : public Layer {
public:
  explicit LayerInventoryContext(sdl2w::Window* _window,
                                 bmin::String itemId,
                                 bmin::String itemName);
  virtual ~LayerInventoryContext() = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

namespace layers {

class LayerMagic : public Layer {
private:
  static ui::PageMagicSetupSpellEntry makeSpellEntry(const db::Database& database,
                                                     const bmin::String& spellName);
  static ui::PageMagicSetupRuneSlot makeRuneSlotFromRuneType(model::RuneType runeType);

public:
  explicit LayerMagic(sdl2w::Window* _window);
  virtual ~LayerMagic() = default;

  void onKeyDown(std::string_view key, int keyCode) override;

  void syncMagicPartyMember();
};

} // namespace layers

namespace layers {

class LayerPickUp : public Layer {
  static constexpr int donePressDurationMs = 200;

  bool isClosing = false;
  bool closeEnqueued = false;
  int donePressElapsedMs = 0;
  std::optional<std::pair<int, int>> containerTile;

  void beginCloseWithDonePress();
  ui::ButtonModal* findDoneButton();

public:
  explicit LayerPickUp(sdl2w::Window* _window);
  LayerPickUp(sdl2w::Window* _window, int containerX, int containerY);
  virtual ~LayerPickUp() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void syncCurrentPartyMember();
};

} // namespace layers

namespace layers {

class LayerPickUpContext : public Layer {
private:
  model::ItemInstance item;

public:
  explicit LayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item);
  virtual ~LayerPickUpContext() = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

namespace layers {

class LayerPopupText : public Layer {
public:
  explicit LayerPopupText(sdl2w::Window* _window, bmin::String title, bmin::String text);
  ~LayerPopupText() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

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

namespace layers {

class LayerSpellInfo : public Layer {
public:
  explicit LayerSpellInfo(sdl2w::Window* _window, const bmin::String& spellName);
  ~LayerSpellInfo() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers

} // export

namespace layers {

LayerDropConfirm::LayerDropConfirm(sdl2w::Window* _window,
                                   bmin::String characterPlayerId,
                                   bmin::String itemId)
    : Layer(_window, state::LayerId::DropConfirm) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (characterPlayerId.empty() || itemId.empty()) {
    LOG(ERROR) << "LayerDropConfirm: characterPlayerId or itemId is empty" << LOG_ENDL;
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerDropConfirm: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }
  auto& player = stateManager->getState().player;
  auto* partyMember = model::playerFindPartyMemberById(player, characterPlayerId);
  if (partyMember == nullptr) {
    LOG(ERROR) << "LayerDropConfirm: party member not found" << LOG_ENDL;
    remove();
    return;
  }

  model::ItemInstance itemInstance;
  for (const auto& item : partyMember->inventory) {
    if (item.id == itemId) {
      itemInstance.id = item.id;
      itemInstance.itemTemplateName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }
  if (itemInstance.id.empty()) {
    LOG(ERROR) << "LayerDropConfirm: item not found in inventory" << LOG_ENDL;
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();

  auto popup = new ui::PopupDropConfirm(window, nullptr);
  popup->setId("popupDropConfirm");

  ui::PopupDropConfirmProps popupProps;
  popupProps.characterPlayerId = characterPlayerId;
  popupProps.itemId = itemId;
  {
    const auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName));
    popupProps.itemLabel =
        itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  }
  popup->setProps(popupProps);

  popup->setScale(1.f);
  auto [popupW, popupH] = popup->getDims();
  popup->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popup->build();

  addUiElement(popup);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerDropConfirm::update(int deltaTime) { Layer::update(deltaTime); }

void LayerDropConfirm::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

namespace layers {

LayerEquipRunes::LayerEquipRunes(sdl2w::Window* _window,
                                 const bmin::String& _characterPlayerId)
    : Layer(_window, state::LayerId::EquipRunes), characterPlayerId(_characterPlayerId) {
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
  addUiElement(minipage);

  syncFromCharacter();

  subscribeAction<state::actions::UiAdjustEquippedRune>(
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
  stateManager->enqueueAction(stateManager->getActionData(),
                              new state::actions::UiCancelEquipRunes(),
                              0);
}

} // namespace layers

namespace layers {

LayerGiveContext::LayerGiveContext(sdl2w::Window* _window,
                                   bmin::String fromCharacterPlayerId,
                                   bmin::String itemId)
    : Layer(_window, state::LayerId::GiveContext) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (fromCharacterPlayerId.empty() || itemId.empty()) {
    LOG(ERROR) << "LayerGiveContext: fromCharacterPlayerId or itemId is empty"
               << LOG_ENDL;
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerGiveContext: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }
  auto& player = stateManager->getState().player;
  auto* fromMember = model::playerFindPartyMemberById(player, fromCharacterPlayerId);
  if (fromMember == nullptr) {
    LOG(ERROR) << "LayerGiveContext: from party member not found" << LOG_ENDL;
    remove();
    return;
  }

  model::ItemInstance itemInstance;
  for (const auto& item : fromMember->inventory) {
    if (item.id == itemId) {
      itemInstance.id = item.id;
      itemInstance.itemTemplateName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }
  if (itemInstance.id.empty()) {
    LOG(ERROR) << "LayerGiveContext: item not found in inventory" << LOG_ENDL;
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();

  auto popupGive = new ui::PopupGive(window, nullptr);
  popupGive->setId("popupGive");

  ui::PopupGiveProps popupProps;
  popupProps.fromCharacterPlayerId = fromCharacterPlayerId;
  popupProps.itemId = itemId;
  {
    const auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName));
    popupProps.itemLabel =
        itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  }
  popupProps.maxQuantity = itemInstance.quantity;
  popupProps.selectedQuantity = std::max(1, itemInstance.quantity);
  popupProps.showQuantitySlider =
      database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName)).stackable &&
      itemInstance.quantity > 1;
  for (const auto& member : player.party) {
    const auto label = member.params.label.empty() ? member.name : member.params.label;
    popupProps.partyMembers.pushBack(
        {.characterPlayerId = member.instanceId,
         .label = label,
         .spriteName = model::characterPlayerGetSprite(member)});
  }
  popupGive->setProps(popupProps);

  popupGive->setScale(1.f);
  auto [popupW, popupH] = popupGive->getDims();
  popupGive->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupGive->build();

  addUiElement(popupGive);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerGiveContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerGiveContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

namespace layers {

LayerInventory::LayerInventory(sdl2w::Window* _window) : Layer(_window, state::LayerId::Inventory) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto pageInventory = new ui::PageInventory(window);
  pageInventory->setId("pageInventory");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  pageInventory->setPos(0, 0);
  pageInventory->setScale(scale);
  auto pageInitProps = pageInventory->getProps();
  // Window dims; PageInventory → ModalStandard default CappedCentered fits/centers.
  pageInitProps.width = static_cast<int>(windowWidth / scale);
  pageInitProps.height = static_cast<int>(windowHeight / scale);
  pageInventory->setProps(pageInitProps);

  addUiElement(pageInventory);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncInventoryPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMemberInventory>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiReorderInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiToggleEquipInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiGiveInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiDropInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
}

void LayerInventory::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }

  if (const auto partyIndex = ui::getPartyMemberIndexFromKey(key)) {
    if (*partyIndex <
        static_cast<int>(stateManager->getState().player.party.size())) {
      stateManager->enqueueAction(
          stateManager->getActionData(),
          new state::actions::UiSetCurrentPartyMemberInventory(*partyIndex),
          0);
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(state::LayerId::Inventory),
      0);
}

void LayerInventory::syncInventoryPartyMember() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto& player = stateManager->getState().player;
  auto inventoryPartyMember = model::playerFindPartyMemberByIndex(
      player, player.currentPartyMemberInventoryIndex);

  if (!inventoryPartyMember) {
    LOG(ERROR) << "LayerInventory::syncInventoryPartyMember: party member is nullptr"
               << LOG_ENDL;
    remove();
    return;
  }

  auto pageInventory = getUiElement<ui::PageInventory>("pageInventory");
  if (!pageInventory) {
    LOG(ERROR) << "LayerInventory::syncInventoryPartyMember: pageInventory is nullptr"
               << LOG_ENDL;
    return;
  }

  auto pageProps = pageInventory->getProps();
  pageProps.characterPlayerId = inventoryPartyMember->instanceId;
  pageProps.characterPlayerLabel = inventoryPartyMember->params.label;
  pageProps.partyMemberInventoryIndex = player.currentPartyMemberInventoryIndex;
  pageProps.partyMembers.clear();
  for (const auto& member : player.party) {
    pageProps.partyMembers.pushBack(
        {.spriteName = model::characterPlayerGetSprite(member)});
  }
  pageProps.characterPlayerSprite = model::characterPlayerGetSprite(*inventoryPartyMember);
  pageProps.weightCarrying =
      model::characterGetWeightCarrying(*inventoryPartyMember, getDatabase());
  pageProps.weightCapacity = model::characterGetWeightCapacity(*inventoryPartyMember);
  pageProps.gold = player.gold;
  pageProps.inventory = inventoryPartyMember->inventory;
  pageProps.equipment = inventoryPartyMember->equipment;
  pageInventory->setProps(pageProps);
}

} // namespace layers

namespace layers {

LayerInventoryContext::LayerInventoryContext(sdl2w::Window* _window,
                                             bmin::String itemId,
                                             bmin::String itemName)
    : Layer(_window, state::LayerId::InventoryContext) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (itemId.empty() || itemName.empty()) {
    LOG(ERROR) << "LayerInventoryContext::LayerInventoryContext: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }

  auto database = getDatabase();
  auto stateManager = getStateManager();
  auto& player = stateManager->getState().player;
  auto* inventoryPartyMember = model::playerFindPartyMemberByIndex(
      player, player.currentPartyMemberInventoryIndex);

  if (inventoryPartyMember == nullptr) {
    LOG(ERROR) << "LayerInventoryContext::LayerInventoryContext: inventory party "
                  "member is nullptr"
               << LOG_ENDL;
    return;
  }

  model::ItemInstance itemInstance;
  for (const auto& item : inventoryPartyMember->inventory) {
    if (item.id == itemId) {
      itemInstance.id = item.id;
      itemInstance.itemTemplateName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }

  auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName));

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupInventoryItem = new ui::PopupInventoryItem(window, nullptr, orientation);
  popupInventoryItem->setId("popupInventoryItem");

  ui::PopupInventoryItemProps popupProps;
  popupProps.characterPlayerId = inventoryPartyMember->instanceId;
  popupProps.item = {
      .id = itemInstance.id,
      .itemTemplateName = itemInstance.itemTemplateName,
      .quantity = itemInstance.quantity,
  };
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.weight = itemInstance.quantity * itemTemplate.weight;
  popupProps.value = itemInstance.quantity * itemTemplate.value;
  popupProps.orientation = orientation;
  popupInventoryItem->setProps(popupProps);

  auto [popupW, popupH] = popupInventoryItem->getDims();
  popupInventoryItem->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupInventoryItem->setScale(1.0f);
  popupInventoryItem->build();

  addUiElement(popupInventoryItem);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerInventoryContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerInventoryContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

namespace layers {

ui::PageMagicSetupSpellEntry LayerMagic::makeSpellEntry(const db::Database& database,
                                                        const bmin::String& spellName) {
  ui::PageMagicSetupSpellEntry entry;
  entry.id = spellName;

  const auto* spell = database.findSpellTemplate(bmin::toStringView(spellName));
  if (spell == nullptr) {
    LOG(ERROR) << "LayerMagic::makeSpellEntry: spell template not found: " << spellName
               << LOG_ENDL;
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
  // entry.manaCost = model::spellAbilityManaCost(*spell, database);
  entry.manaCost = ability->costValue;
  for (const auto& req : spell->requiredRunes) {
    const auto sprite = model::runeTypeToSpriteName(req.type);
    const int count = req.count > 0 ? req.count : 1;
    for (int i = 0; i < count; ++i) {
      entry.requiredRuneSprites.push_back(sprite);
    }
  }
  return entry;
}

ui::PageMagicSetupRuneSlot LayerMagic::makeRuneSlotFromRuneType(
    model::RuneType runeType) {
  ui::PageMagicSetupRuneSlot slot{.filled = true};
  slot.iconSprite = model::runeTypeToSpriteName(runeType);
  return slot;
}

LayerMagic::LayerMagic(sdl2w::Window* _window) : Layer(_window, state::LayerId::Magic) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto pageMagicSetup = new ui::PageMagicSetup(window);
  pageMagicSetup->setId("pageMagicSetup");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  pageMagicSetup->setPos(0, 0);
  pageMagicSetup->setScale(scale);
  auto pageInitProps = pageMagicSetup->getProps();
  // Window dims; PageMagicSetup → ModalStandard default CappedCentered fits/centers.
  pageInitProps.width = static_cast<int>(windowWidth / scale);
  pageInitProps.height = static_cast<int>(windowHeight / scale);
  pageMagicSetup->setProps(pageInitProps);

  addUiElement(pageMagicSetup);

  syncMagicPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMemberMagic>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::actions::UiSetSpellReady>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::actions::UiCommitEquipRunes>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::actions::UiCancelEquipRunes>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
}

void LayerMagic::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }

  if (const auto partyIndex = ui::getPartyMemberIndexFromKey(key)) {
    if (*partyIndex < static_cast<int>(stateManager->getState().player.party.size())) {
      stateManager->enqueueAction(
          stateManager->getActionData(),
          new state::actions::UiSetCurrentPartyMemberMagic(*partyIndex),
          0);
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(state::LayerId::Magic),
      0);
}

void LayerMagic::syncMagicPartyMember() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerMagic::syncMagicPartyMember: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }

  auto& player = stateManager->getState().player;
  auto magicPartyMember =
      model::playerFindPartyMemberByIndex(player, player.currentPartyMemberMagicIndex);

  if (!magicPartyMember) {
    LOG(ERROR) << "LayerMagic::syncMagicPartyMember: party member is nullptr" << LOG_ENDL;
    remove();
    return;
  }

  auto pageMagicSetup = getUiElement<ui::PageMagicSetup>("pageMagicSetup");
  if (!pageMagicSetup) {
    LOG(ERROR) << "LayerMagic::syncMagicPartyMember: pageMagicSetup is nullptr"
               << LOG_ENDL;
    return;
  }

  auto pageProps = pageMagicSetup->getProps();
  pageProps.characterPlayerId = magicPartyMember->instanceId;
  pageProps.characterPlayerLabel = magicPartyMember->params.label;
  pageProps.partyMemberMagicIndex = player.currentPartyMemberMagicIndex;
  pageProps.partyMembers.clear();
  for (const auto& member : player.party) {
    pageProps.partyMembers.pushBack(
        {.spriteName = model::characterPlayerGetSprite(member)});
  }
  pageProps.characterPlayerSprite = model::characterPlayerGetSprite(*magicPartyMember);

  pageProps.selectedRuneSlotIndex = -1;

  pageProps.elementCounts.clear();
  for (int i = 0; i < model::kRuneTypeCount; ++i) {
    const auto runeType = model::runeTypeFromIndex(i);
    const int available =
        model::characterPlayerCountAvailableRunesOfType(*magicPartyMember, runeType);
    const int equipped =
        model::characterPlayerCountEquippedRunesOfType(*magicPartyMember, runeType);
    pageProps.elementCounts.pushBack(ui::PageMagicSetupElementCount{
        .iconSprite = model::runeTypeToSpriteName(runeType),
        // Remaining unequipped (same as Equip Runes modal center counts).
        .count = available > equipped ? available - equipped : 0,
    });
  }

  // Single spell list; ready = equipped runes meet requiredRunes (shown as "(r)").
  pageProps.spells.clear();
  // TODO fix spell list
  for (const auto& spellName : magicPartyMember->knownSpells) {
    auto entry = makeSpellEntry(*database, spellName);
    entry.ready = true;
    pageProps.spells.pushBack(entry);
  }

  pageProps.runeSlots.clear();
  for (size_t i = 0; i < model::CharacterPlayer::kRuneSlotCount; ++i) {
    if (i < magicPartyMember->equippedRunes.size()) {
      pageProps.runeSlots.pushBack(
          makeRuneSlotFromRuneType(magicPartyMember->equippedRunes[i]));
    } else {
      pageProps.runeSlots.pushBack(ui::PageMagicSetupRuneSlot{.filled = false});
    }
  }

  pageMagicSetup->setProps(pageProps);
}

} // namespace layers

namespace layers {

ui::ButtonModal* LayerPickUp::findDoneButton() {
  auto* minipagePickUp = getUiElement<ui::MinipagePickUp>("minipagePickUp");
  if (!minipagePickUp) {
    return nullptr;
  }
  auto* modal = minipagePickUp->getChildById("modal");
  if (!modal) {
    return nullptr;
  }
  auto* buttonGroup = modal->getChildById("buttonGroup");
  if (!buttonGroup) {
    return nullptr;
  }
  return dynamic_cast<ui::ButtonModal*>(buttonGroup->getChildById("buttonGroupButton_0"));
}

void LayerPickUp::beginCloseWithDonePress() {
  if (isClosing) {
    return;
  }
  isClosing = true;
  donePressElapsedMs = 0;
  if (auto* doneButton = findDoneButton()) {
    doneButton->isActive = true;
  }
}

LayerPickUp::LayerPickUp(sdl2w::Window* _window) : Layer(_window, state::LayerId::PickUp) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto minipagePickUp = new ui::MinipagePickUp(window);
  minipagePickUp->setId("minipagePickUp");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  // Window dims + ModalSmall default CappedCentered (see ui::computeCappedCenteredRect).
  minipagePickUp->setPos(0, 0);
  minipagePickUp->setScale(scale);
  auto minipageInitProps = minipagePickUp->getProps();
  minipageInitProps.width = static_cast<int>(windowWidth / scale);
  minipageInitProps.height = static_cast<int>(windowHeight / scale);
  minipagePickUp->setProps(minipageInitProps);

  addUiElement(minipagePickUp);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncCurrentPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMember>(
      [this](auto&, auto&) { syncCurrentPartyMember(); });
  subscribeAction<state::actions::UiPickUpItem>(
      [this](auto&, auto&) { syncCurrentPartyMember(); });
}

LayerPickUp::LayerPickUp(sdl2w::Window* _window, int containerX, int containerY)
    : LayerPickUp(_window) {
  containerTile = std::make_pair(containerX, containerY);
  syncCurrentPartyMember();
}

void LayerPickUp::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON || isClosing) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }

  if (const auto partyIndex = ui::getPartyMemberIndexFromKey(key)) {
    if (*partyIndex <
        static_cast<int>(stateManager->getState().player.party.size())) {
      stateManager->enqueueAction(
          stateManager->getActionData(),
          new state::actions::UiSetCurrentPartyMember(*partyIndex),
          0);
    }
    return;
  }

  if (const auto itemIndex = ui::getPickUpItemIndexFromKey(key)) {
    auto* minipagePickUp = getUiElement<ui::MinipagePickUp>("minipagePickUp");
    if (minipagePickUp &&
        *itemIndex <
            static_cast<int>(minipagePickUp->getProps().nearbyItems.size())) {
      const auto& item = minipagePickUp->getProps().nearbyItems[*itemIndex];
      stateManager->enqueueAction(stateManager->getActionData(),
                                  new state::actions::UiPickUpItem(item.id),
                                  0);
    }
    return;
  }

  if (ui::isCancelActionKey(key) || ui::isConfirmActionKey(key)) {
    beginCloseWithDonePress();
  }
}

void LayerPickUp::syncCurrentPartyMember() {
  if (isClosing) {
    return;
  }
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto database = getDatabase();
  if (!stateManager || !database) {
    remove();
    return;
  }

  auto& state = stateManager->getState();
  auto& player = state.player;
  auto currentPartyMember =
      model::playerFindPartyMemberByIndex(player, player.currentPartyMemberIndex);

  if (!currentPartyMember) {
    LOG(ERROR) << "LayerPickUp::syncCurrentPartyMember: party member is nullptr"
               << LOG_ENDL;
    remove();
    return;
  }

  auto minipagePickUp = getUiElement<ui::MinipagePickUp>("minipagePickUp");
  if (!minipagePickUp) {
    LOG(ERROR) << "LayerPickUp::syncCurrentPartyMember: minipagePickUp is nullptr"
               << LOG_ENDL;
    return;
  }

  const int carrying = model::characterGetWeightCarrying(*currentPartyMember, database);
  const int maxWeight = model::characterGetWeightCapacity(*currentPartyMember);

  auto minipageProps = minipagePickUp->getProps();
  minipageProps.doneButtonRemoveLayerId = strutil::fromStringView(state::layerIdString(state::LayerId::PickUp));
  minipageProps.partyMemberIndex = player.currentPartyMemberIndex;
  minipageProps.partyMemberSprites.clear();
  for (const auto& member : player.party) {
    minipageProps.partyMemberSprites.pushBack(model::characterPlayerGetSprite(member));
  }
  minipageProps.weightText = bmin::String(TRANSLATE("Carrying")) + " " +
                             bmin::toString(carrying) + "/" + bmin::toString(maxWeight);

  minipageProps.nearbyItems.clear();
  if (containerTile) {
    minipageProps.titleText = TRANSLATE("Container");
    minipageProps.nearbyItems = game::collectItemsAtActiveMapTile(
        state.world.activeMap, containerTile->first, containerTile->second);
    if (minipageProps.nearbyItems.empty()) {
      minipageProps.statusText = TRANSLATE("Nothing inside.");
    } else {
      minipageProps.statusText.clear();
    }
  } else {
    minipageProps.titleText = TRANSLATE("Pick Up");
    if (const auto* avatar = game::findDropCharacterOnActiveMap(
            state.world.activeMap, player, currentPartyMember->instanceId)) {
      minipageProps.nearbyItems = game::collectItemsWithinPickupRange(
          state.world.activeMap, *avatar, game::PICKUP_PATH_RANGE, *database);
    }
    if (minipageProps.nearbyItems.empty()) {
      minipageProps.statusText = TRANSLATE("No items nearby.");
    } else {
      minipageProps.statusText.clear();
    }
  }

  minipagePickUp->setProps(minipageProps);
}

void LayerPickUp::update(int deltaTime) {
  Layer::update(deltaTime);
  if (!isClosing || closeEnqueued) {
    return;
  }

  // Keep Done visually pressed while the close delay runs.
  if (auto* doneButton = findDoneButton()) {
    doneButton->isActive = true;
  }

  donePressElapsedMs += deltaTime;
  if (donePressElapsedMs < donePressDurationMs) {
    return;
  }

  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  closeEnqueued = true;
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(state::LayerId::PickUp),
      0);
}

} // namespace layers

namespace layers {

LayerPickUpContext::LayerPickUpContext(sdl2w::Window* _window,
                                       const model::ItemInstance& item)
    : Layer(_window), item(item) {

  if (!assertInterfaces()) {
    remove();
    return;
  }
  if (item.itemTemplateName.empty() || item.id.empty()) {
    LOG(ERROR) << "LayerPickUpContext::LayerPickUpContext: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }
  auto database = getDatabase();
  auto& itemTemplate = database->getItemTemplate(bmin::toStringView(item.itemTemplateName));

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupPickupItem = new ui::PopupPickupItem(window, getId(), orientation);
  popupPickupItem->setId("popupPickupItem");

  ui::PopupPickupItemProps popupProps;
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.weight = item.quantity * itemTemplate.weight;
  popupProps.value = item.quantity * itemTemplate.value;
  popupProps.orientation = orientation;
  popupPickupItem->setProps(popupProps);

  auto [popupW, popupH] = popupPickupItem->getDims();
  popupPickupItem->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupPickupItem->setScale(1.0f);
  popupPickupItem->build();

  addUiElement(popupPickupItem);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerPickUpContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerPickUpContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

namespace layers {

namespace {

constexpr int POPUP_WIDTH = 300;
constexpr int PADDING = 8;
constexpr int CLOSE_BUTTON_PADDING = 4;

} // namespace

LayerPopupText::LayerPopupText(sdl2w::Window* _window,
                               bmin::String title,
                               bmin::String text)
    : Layer(_window, state::LayerId::PopupText) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (text.empty()) {
    LOG(ERROR) << "LayerPopupText: text is empty" << LOG_ENDL;
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();
  const int closeButtonSize = ui::ButtonClose::closeButtonSize;
  const int headerY = CLOSE_BUTTON_PADDING;
  const int titleWidth =
      POPUP_WIDTH - 2 * PADDING - closeButtonSize - CLOSE_BUTTON_PADDING;
  const int bodyWidth = POPUP_WIDTH - 2 * PADDING;

  auto* titleLine = new ui::TextLine(window, nullptr);
  titleLine->setId("title");
  titleLine->setPos(PADDING, headerY + closeButtonSize / 2);
  titleLine->setScale(1.f);
  ui::TextFontProps titleFont;
  ui::setBaseFontConfig(titleFont, ui::BaseFontConfig::MODAL_TITLE);
  ui::TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = ui::Colors::Black;
  titleProps.textAlign = ui::TextAlign::LEFT_CENTER;
  titleProps.textBlocks.pushBack({.text = title});
  titleLine->setProps(titleProps);
  auto [_, titleHeight] = titleLine->calculateTextDims();
  const int headerHeight = std::max(titleHeight, closeButtonSize);
  int contentY = headerY + headerHeight + PADDING;

  auto* paragraph = new ui::TextParagraph(window, nullptr);
  paragraph->setId("helpText");
  paragraph->setPos(PADDING, contentY);
  paragraph->setScale(1.f);
  ui::TextFontProps paragraphFont;
  ui::setBaseFontConfig(paragraphFont, ui::BaseFontConfig::MODAL_TEXT);
  ui::TextParagraphProps paragraphProps;
  paragraphProps.width = bodyWidth;
  paragraphProps.fontFamily = paragraphFont.fontFamily;
  paragraphProps.fontSize = paragraphFont.fontSize;
  paragraphProps.fontColor = ui::Colors::Black;
  paragraphProps.textAlign = ui::TextAlign::LEFT_TOP;
  paragraphProps.lineSpacing = 0;
  paragraphProps.padding = 0;
  paragraphProps.textBlocks.pushBack({.text = text});
  paragraph->setProps(paragraphProps);
  auto [__, messageHeight] = paragraph->getDims();
  contentY += messageHeight;

  const int popupHeight = contentY + PADDING;
  const int popupX = (windowWidth - POPUP_WIDTH) / 2;
  const int popupY = (windowHeight - popupHeight) / 2;

  auto* root = new ui::UiElement(window, nullptr);
  root->setId("popupTextRoot");
  root->setPos(0, 0);
  root->setScale(1.f);

  auto* backdrop = new ui::Quad(window, root);
  backdrop->setId("backdrop");
  backdrop->setPos(0, 0);
  backdrop->setScale(1.f);
  backdrop->setProps(ui::QuadProps{
      .width = windowWidth,
      .height = windowHeight,
      .bgColor = ui::Colors::Transparent,
  });
  backdrop->addEventObserver(new ui::ObserverRemoveLayer(state::LayerId::PopupText));
  root->addChild(backdrop);

  auto* border = new ui::BorderDropShadow(window, root);
  border->setId("border");
  border->setPos(popupX, popupY);
  border->setScale(1.f);
  border->setProps(ui::BorderDropShadowProps{
      .width = POPUP_WIDTH,
      .height = popupHeight,
      .backgroundColor = ui::Colors::White,
      .shadowColor = ui::Colors::Black,
      .borderSize = 2,
  });
  border->addChild(titleLine);
  border->addChild(paragraph);
  root->addChild(border);

  auto* closeButton = new ui::ButtonClose(window, root);
  closeButton->setId("closeButton");
  closeButton->setPos(popupX + POPUP_WIDTH - closeButtonSize - CLOSE_BUTTON_PADDING,
                      popupY + CLOSE_BUTTON_PADDING);
  closeButton->setScale(1.f);
  closeButton->setProps(ui::ButtonCloseProps{.closeType = ui::CloseType::POPUP});
  closeButton->addEventObserver(new ui::ObserverRemoveLayer(state::LayerId::PopupText));
  root->addChild(closeButton);

  addUiElement(root);

  auto* floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerPopupText::update(int deltaTime) { Layer::update(deltaTime); }

void LayerPopupText::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

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

namespace layers {

LayerSpellInfo::LayerSpellInfo(sdl2w::Window* _window, const bmin::String& spellName)
    : Layer(_window, state::LayerId::SpellInfo) {
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
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(state::LayerId::SpellInfo),
      0);
}

} // namespace layers
