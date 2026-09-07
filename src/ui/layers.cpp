module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <optional>

module carcer.ui.layers;
import carcer.ui.screens.runtime;
import carcer.ui.widgets;
import carcer.model;
import carcer.data;
import carcer.state;
import carcer.ui.screens.layouts;
import carcer.ui.screens.overlays;
import carcer.ui.screens.pages;
import carcer.db;
import carcer.in3;
import sdl2w;
import carcer.ui.core;
import bmin.containers;
import bmin.string_interop;
import carcer.actions;
import carcer.game.combat;
import carcer.game.map;
import carcer.game.inventory;
import carcer.lib.StringUtil;
#include "macros.h"

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

class LayerSpellInfo : public Layer {
public:
  explicit LayerSpellInfo(sdl2w::Window* _window, const bmin::String& spellName);
  ~LayerSpellInfo() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

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
      state::actions::removeLayer(state::LayerId::SpellInfo),
      0);
}

} // namespace layers


namespace layers {

class LayerWorld : public Layer {
private:
  void attachWorldActionObservers(ui::InGameLayout* inGameLayout);
  void attachPartyMemberObservers(ui::InGameLayout* inGameLayout);
  void syncWorldActionModeHighlight();
  void syncActionModeCancelButton();
  void syncCombatTitleBar();
  void updateHeldMoveRepeat(int deltaTime);
  void confirmWorldActionAim(int tileX, int tileY);
  void updateAimFromMouse(int x, int y);
  static bool canPlayerIssueCombatMove(const state::State& state);
  static void enqueueMapMove(state::StateManager& stateManager, int dx, int dy);
  static void enqueueCombatWait(state::StateManager& stateManager);
  static void ensureCurrentPartyMemberSelection(state::State& state);
  float mapScale = 1.f;
  bool hasLastMousePos = false;
  int lastMouseX = 0;
  int lastMouseY = 0;

  void alignMapView();
  void setWorldActionTypes(model::TurnMode turnMode,
                           bmin::DynArray<state::WorldActionType>& dest);

public:
  explicit LayerWorld(sdl2w::Window* _window);
  virtual ~LayerWorld() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void onMouseDown(int x, int y, int button) override;
  void onMouseHover(int x, int y) override;
  void syncFromState();
  void setMapScale(float scale);
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers


namespace layers {

LayerWorld::LayerWorld(sdl2w::Window* _window) : Layer(_window, state::LayerId::World) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto inGameLayout = new ui::InGameLayout(window);
  inGameLayout->setId("inGameLayout");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  inGameLayout->setPos(0, 0);
  inGameLayout->setScale(scale);

  auto layoutInitProps = inGameLayout->getProps();
  layoutInitProps.width = static_cast<int>(windowWidth / scale);
  layoutInitProps.height = static_cast<int>(windowHeight / scale);
  layoutInitProps.actionButtonScale = 1.5f;
  layoutInitProps.borderType = ui::InGameBorderType::Wide;
  inGameLayout->setProps(layoutInitProps);

  auto titleBar = new ui::InGameTitleBar(window);
  titleBar->setProps(ui::InGameTitleBarProps{
      .title = "World",
      .day = 0,
      .food = 0,
      .ap = 0,
      .showAp = false,
  });
  inGameLayout->setTitleElement(titleBar);

  // Map under action buttons: layer draws uiElements in order, so MapView first.
  auto mapView = new ui::MapView(window);
  mapView->setId("mapView");
  alignMapView();
  addUiElement(mapView);
  addUiElement(inGameLayout);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  subscribeAction<state::ActionEvent::StartCombat>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::EndCombat>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::SetActiveCombatCharacter>(
      [this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::UiSetSelectedPartyMemberId>(
      [this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::ModifyHP>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::ModifyPartyMemberHp>(
      [this](auto&, auto&) { syncFromState(); });

  syncFromState();
}

bool LayerWorld::canPlayerIssueCombatMove(const state::State& state) {
  const auto& world = state.world;
  if (!world.combat.active || !world.combat.isWaitingForAction) {
    return false;
  }
  return model::isPartyMember(state.player, world.combat.activeCharacterId);
}

void LayerWorld::enqueueMapMove(state::StateManager& stateManager, int dx, int dy) {
  auto& state = stateManager.getState();
  if (state.world.combat.active) {
    if (!canPlayerIssueCombatMove(state)) {
      return;
    }
    stateManager.enqueueAction(
        stateManager.getActionData(),
        state::actions::doCombatAction(state.world.combat.activeCharacterId,
                                           model::CombatActionType::MOVE,
                                           {.targetLoc = {dx, dy}}),
        0);
    return;
  }
  if (state.world.resolvingTownEnemyAi) {
    return;
  }
  stateManager.enqueueAction(
      stateManager.getActionData(), state::actions::movePlayer(dx, dy), 0);
}

void LayerWorld::enqueueCombatWait(state::StateManager& stateManager) {
  if (!canPlayerIssueCombatMove(stateManager.getState())) {
    return;
  }
  stateManager.enqueueAction(stateManager.getActionData(),
                             state::actions::doCombatAction(
                                 stateManager.getState().world.combat.activeCharacterId,
                                 model::CombatActionType::WAIT),
                             0);
}

void LayerWorld::ensureCurrentPartyMemberSelection(state::State& state) {
  auto& player = state.player;
  if (player.party.empty()) {
    state.uiState.selectedPartyMemberId.clear();
    return;
  }

  // UI selection only — never tied to map movement / party avatar.
  if (model::playerFindPartyMemberIndexById(player,
                                            state.uiState.selectedPartyMemberId) >= 0) {
    return;
  }
  state.uiState.selectedPartyMemberId = player.party[0].instanceId;
}

void LayerWorld::syncCombatTitleBar() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    return;
  }
  auto* titleBar = dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement());
  if (!titleBar) {
    return;
  }

  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& state = stateManager->getState();
  auto& world = state.world;
  game::ActiveMapOrchestrator activeMap(
      world.activeMap, state.mapInstances, getDatabase());
  auto titleProps = titleBar->getProps();
  const bool showAp = world.combat.active;
  int ap = 0;
  if (showAp) {
    if (const auto* character =
            activeMap.findCharacterById(world.combat.activeCharacterId)) {
      ap = character->currentAp;
    }
  }
  if (titleProps.showAp != showAp || titleProps.ap != ap) {
    titleProps.showAp = showAp;
    titleProps.ap = ap;
    titleBar->setProps(titleProps);
  }
}

void LayerWorld::confirmWorldActionAim(int tileX, int tileY) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  if (stateManager->getState().world.resolvingTownEnemyAi) {
    return;
  }
  const auto actionMode = stateManager->getState().world.actionMode;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(stateManager->getActionData(),
                                state::actions::examineAt(window, tileX, tileY),
                                0);
    return;
  }
  if (actionMode == model::WorldActionMode::TALK) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::talkAt(tileX, tileY), 0);
    return;
  }
  if (actionMode != model::WorldActionMode::SPELL) {
    return;
  }

  auto& state = stateManager->getState();
  auto& world = state.world;
  if (!canPlayerIssueCombatMove(state)) {
    return;
  }
  if (world.pendingSpellId.empty()) {
    return;
  }

  auto* database = getDatabase();
  if (database == nullptr) {
    return;
  }
  const auto* spell =
      database->findSpellTemplate(bmin::toStringView(world.pendingSpellId));
  if (spell == nullptr) {
    return;
  }
  const auto* ability =
      database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
  if (ability == nullptr) {
    return;
  }

  const model::CharacterInstance* caster = nullptr;
  for (const auto& character : world.activeMap.characters) {
    if (character.id == world.combat.activeCharacterId) {
      caster = &character;
      break;
    }
  }
  if (caster == nullptr) {
    return;
  }

  const int distance = game::chebyshevDistance(caster->x, caster->y, tileX, tileY);
  if (distance > ability->targetSelect.range) {
    return;
  }

  ui::setHeldMoveActive(*stateManager, false);
  stateManager->enqueueAction(
      stateManager->getActionData(),
      state::actions::doCombatAction(
          world.combat.activeCharacterId,
          model::CombatActionType::SPELL,
          {.abilityId = world.pendingSpellId, .targetLoc = {tileX, tileY}}),
      0);
  stateManager->pllAction(
      stateManager->getActionData(),
      state::actions::setActionMode(model::WorldActionMode::NONE),
      0);
}

void LayerWorld::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& world = stateManager->getState().world;

  if (ui::isCancelActionKey(key)) {
    bmin::List<model::WorldActionMode> cancellableActionModes = {
        model::WorldActionMode::EXAMINE,
        model::WorldActionMode::TALK,
        model::WorldActionMode::SPELL,
    };
    if (cancellableActionModes.contains(world.actionMode)) {
      ui::cancelCurrentWorldActionMode(*stateManager);
      return;
    }
  }

  if (ui::isCombatWaitKey(key) && world.combat.active) {
    ui::setHeldMoveActive(*stateManager, false);
    enqueueCombatWait(*stateManager);
    return;
  }

  // Block world-action shortcuts / aim confirm while town AI is resolving.
  if (!world.resolvingTownEnemyAi) {
    // `r` always opens magic setup (not remapped through combat Ability → cast).
    if (ui::isOpenMagicSetupKey(key)) {
      ui::showMagicSetupLayer(*stateManager, window);
      return;
    }
    if (ui::isOpenSpellCastKey(key)) {
      ui::showSpellCastLayer(*stateManager, window);
      return;
    }
    if (auto actionType = ui::getWorldActionFromKeyboardShortcut(
            key, stateManager->getState().turnMode)) {
      ui::activateWorldAction(*stateManager, *actionType, window);
      return;
    }
  }

  const bool isAimMode = world.actionMode == model::WorldActionMode::EXAMINE ||
                         world.actionMode == model::WorldActionMode::TALK ||
                         world.actionMode == model::WorldActionMode::SPELL;

  if (isAimMode && ui::isConfirmActionKey(key)) {
    if (!world.resolvingTownEnemyAi && world.actionAimTile) {
      confirmWorldActionAim(world.actionAimTile->x, world.actionAimTile->y);
    }
    return;
  }

  auto moveDelta = ui::getMoveDeltaForKey(key);
  if (!moveDelta) {
    return;
  }

  if (isAimMode) {
    if (world.resolvingTownEnemyAi) {
      return;
    }
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::moveActionAim(moveDelta->dx, moveDelta->dy),
        0);
    return;
  }

  const auto& heldMove = stateManager->getState().uiState.heldMove;
  // Ignore OS/SDL key-repeat events for the same held key; we time repeats ourselves.
  if (heldMove.isActive && heldMove.key.sliceView() == key) {
    return;
  }

  const bool canEnqueueMove =
      !world.resolvingTownEnemyAi &&
      (!world.combat.active || canPlayerIssueCombatMove(stateManager->getState()));

  state::HeldMove nextHeldMove{
      .isActive = true,
      .key = bmin::fromStringView(key),
      .dx = moveDelta->dx,
      .dy = moveDelta->dy,
  };
  model::timerStructRestart(nextHeldMove.initialDelay);
  model::timerStructRestart(nextHeldMove.moveDelay);
  stateManager->pllAction(stateManager->getActionData(),
                          state::actions::updateHeldMove(nextHeldMove),
                          0);
  if (canEnqueueMove) {
    enqueueMapMove(*stateManager, moveDelta->dx, moveDelta->dy);
  }
}

void LayerWorld::onKeyUp(std::string_view key, int /*keyCode*/) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  const auto& heldMove = stateManager->getState().uiState.heldMove;
  if (heldMove.isActive && heldMove.key.sliceView() == key) {
    ui::setHeldMoveActive(*stateManager, false);
  }
}

void LayerWorld::updateAimFromMouse(int x, int y) {
  if (getState() != LayerState::ON) {
    return;
  }
  // Only react to actual mouse movement so keyboard aim isn't fought by a
  // stationary cursor still sitting over the map.
  if (hasLastMousePos && x == lastMouseX && y == lastMouseY) {
    return;
  }
  hasLastMousePos = true;
  lastMouseX = x;
  lastMouseY = y;

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  const auto actionMode = stateManager->getState().world.actionMode;
  if (actionMode != model::WorldActionMode::EXAMINE &&
      actionMode != model::WorldActionMode::TALK &&
      actionMode != model::WorldActionMode::SPELL) {
    return;
  }
  auto* mapView = getUiElement<ui::MapView>("mapView");
  if (!mapView) {
    return;
  }
  auto tile = mapView->screenToTile(x, y);
  if (!tile) {
    return;
  }
  const auto& aim = stateManager->getState().world.actionAimTile;
  if (aim && aim->x == tile->x && aim->y == tile->y) {
    return;
  }
  // Parallel so hover stays responsive even if sequential actions are waiting.
  stateManager->pllAction(stateManager->getActionData(),
                          state::actions::setActionAim(tile->x, tile->y),
                          0);
}

void LayerWorld::onMouseHover(int x, int y) {
  updateAimFromMouse(x, y);
  Layer::onMouseHover(x, y);
}

void LayerWorld::onMouseDown(int x, int y, int button) {
  // SDL_BUTTON_LEFT == 1
  if (getState() == LayerState::ON && button == 1) {
    auto* stateManager = getStateManager();
    if (stateManager) {
      const auto actionMode = stateManager->getState().world.actionMode;
      if (actionMode == model::WorldActionMode::EXAMINE ||
          actionMode == model::WorldActionMode::TALK ||
          actionMode == model::WorldActionMode::SPELL) {
        if (auto* mapView = getUiElement<ui::MapView>("mapView")) {
          if (auto tile = mapView->screenToTile(x, y)) {
            stateManager->enqueueAction(
                stateManager->getActionData(),
                state::actions::setActionAim(tile->x, tile->y),
                0);
            confirmWorldActionAim(tile->x, tile->y);
            return;
          }
        }
      }
    }
  }
  Layer::onMouseDown(x, y, button);
}

void LayerWorld::alignMapView() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  auto mapView = getUiElement<ui::MapView>("mapView");
  auto world = &getStateManager()->getState().world;
  if (!inGameLayout || !mapView || !world) {
    return;
  }
  auto [worldX, worldY] = inGameLayout->getWorldLocation();
  auto [worldW, worldH] = inGameLayout->getWorldDims();
  // getWorldLocation/Dims are already in screen pixels (scaled).
  mapView->setPos(worldX, worldY);
  mapView->setScale(mapScale);
  mapView->setProps(ui::MapViewProps{
      .width = static_cast<int>(worldW / mapScale),
      .height = static_cast<int>(worldH / mapScale),
  });
  // Content dims are in map pixels; screen size is content * mapScale.
  world->camera.viewW = static_cast<int>(worldW / mapScale);
  world->camera.viewH = static_cast<int>(worldH / mapScale);
}

void LayerWorld::setMapScale(float scale) {
  mapScale = scale;
  alignMapView();
}

void LayerWorld::setWorldActionTypes(model::TurnMode turnMode,
                                     bmin::DynArray<state::WorldActionType>& dest) {
  auto copyActionTypes = [](bmin::DynArray<state::WorldActionType>& dest,
                            const auto& source) {
    dest.clear();
    for (const auto& type : source) {
      dest.pushBack(type);
    }
  };

  const state::WorldActionUiState worldActionUiState;
  switch (turnMode) {
  case model::TurnMode::TURN_OUTDOOR:
    copyActionTypes(dest, worldActionUiState.outdoorModeActionTypes);
    break;
  case model::TurnMode::TURN_COMBAT:
    copyActionTypes(dest, worldActionUiState.townModeFightActionTypes);
    break;
  case model::TurnMode::TURN_TOWN:
  default:
    copyActionTypes(dest, worldActionUiState.townModeActionTypes);
    break;
  }
}

void LayerWorld::attachWorldActionObservers(ui::InGameLayout* inGameLayout) {
  if (!inGameLayout) {
    return;
  }

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    button->addEventObserver(new ui::ObserverWorldAction(
        stateManager, button->getProps().worldActionType, window));
  }
}

void LayerWorld::attachPartyMemberObservers(ui::InGameLayout* inGameLayout) {
  if (!inGameLayout) {
    return;
  }

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto* chList = inGameLayout->getChildById("chList");
  if (!chList) {
    return;
  }
  auto* list = chList->getChildById("list");
  if (!list) {
    return;
  }

  const auto& party = stateManager->getState().player.party;
  auto& children = list->getChildren();
  for (size_t i = 0; i < children.size() && i < party.size(); i++) {
    children[i]->addEventObserver(
        new ui::ObserverSetSelectedPartyMemberId(party[i].instanceId));
  }
}

void LayerWorld::syncWorldActionModeHighlight() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  auto*layerManager = static_cast<layers::LayerManager*>(state::LayerManagerInterface::getLayerManager());
  const bool inventoryOpen =
      layerManager != nullptr &&
      layerManager->getLayerById(state::LayerId::Inventory) != nullptr;
  const bool magicOpen = layerManager != nullptr &&
                         layerManager->getLayerById(state::LayerId::Magic) != nullptr;
  const bool spellCastOpen =
      layerManager != nullptr &&
      layerManager->getLayerById(state::LayerId::SpellCast) != nullptr;
  const bool pickUpOpen = layerManager != nullptr &&
                          layerManager->getLayerById(state::LayerId::PickUp) != nullptr;
  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    const auto actionType = button->getProps().worldActionType;
    const bool modeSelected =
        (actionType == state::WorldActionType::EXAMINE &&
         actionMode == model::WorldActionMode::EXAMINE) ||
        (actionType == state::WorldActionType::TALK &&
         actionMode == model::WorldActionMode::TALK) ||
        (actionType == state::WorldActionType::INVENTORY && inventoryOpen) ||
        (actionType == state::WorldActionType::ABILITY && (magicOpen || spellCastOpen)) ||
        (actionType == state::WorldActionType::GET && pickUpOpen);
    if (button->isModeSelected != modeSelected) {
      button->isModeSelected = modeSelected;
    }
  }
}

void LayerWorld::syncActionModeCancelButton() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  bmin::String modeLabel;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    modeLabel = TRANSLATE("Examine");
  } else if (actionMode == model::WorldActionMode::TALK) {
    modeLabel = TRANSLATE("Talk");
  } else if (actionMode == model::WorldActionMode::SPELL) {
    modeLabel = TRANSLATE("Cast Spell");
  }

  const bool shouldShow = !modeLabel.empty();
  auto* existingCancel = inGameLayout->getChildById("actionModeCancel");
  auto* existingLabel =
      dynamic_cast<ui::TextLine*>(inGameLayout->getChildById("actionModeLabel"));
  if (shouldShow && existingCancel && existingLabel) {
    const auto& labelProps = existingLabel->getProps();
    if (!labelProps.textBlocks.empty() && labelProps.textBlocks[0].text == modeLabel) {
      return;
    }
  } else if (!shouldShow && !existingCancel && !existingLabel) {
    return;
  }

  inGameLayout->setActionModeCancelVisible(shouldShow, modeLabel);
  if (shouldShow) {
    if (auto* cancelButton = inGameLayout->getChildById("actionModeCancel")) {
      cancelButton->addEventObserver(
          new ui::ObserverCancelWorldActionMode(getStateManager()));
    }
  }
}

void LayerWorld::syncFromState() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    LOG(ERROR) << "LayerWorld::syncFromState: inGameLayout is nullptr" << LOG_ENDL;
    return;
  }

  auto stateManager = getStateManager();
  auto& state = stateManager->getState();
  auto& player = state.player;
  auto& world = state.world;
  game::ActiveMapOrchestrator activeMap(
      world.activeMap, state.mapInstances, getDatabase());

  ensureCurrentPartyMemberSelection(state);

  const int selectedIndex =
      model::playerFindPartyMemberIndexById(player, state.uiState.selectedPartyMemberId);

  auto layoutProps = inGameLayout->getProps();
  setWorldActionTypes(state.turnMode, layoutProps.worldActionTypes);
  layoutProps.partyMembers.clear();
  layoutProps.selectedPartyMemberIndex = selectedIndex >= 0 ? selectedIndex : 0;
  for (int i = 0; i < static_cast<int>(player.party.size()); i++) {
    const auto& member = player.party[i];
    ui::ChCompactInfoProps entry;
    entry.characterSpriteName = model::characterPlayerGetSprite(member);
    entry.hp = member.currentHp;
    entry.mana = member.currentMp;
    entry.isSelected = (i == layoutProps.selectedPartyMemberIndex);
    layoutProps.partyMembers.pushBack(entry);
  }
  inGameLayout->setProps(layoutProps);
  attachWorldActionObservers(inGameLayout);
  attachPartyMemberObservers(inGameLayout);
  syncWorldActionModeHighlight();
  syncActionModeCancelButton();

  if (auto* titleBar =
          dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement())) {
    auto titleProps = titleBar->getProps();
    titleProps.title = bmin::String("World");
    // day/ap placeholders until those fields live on State
    titleProps.day = 0;
    titleProps.food = player.food;
    titleProps.ap = 0;
    if (world.combat.active) {
      if (const auto* character =
              activeMap.findCharacterById(world.combat.activeCharacterId)) {
        titleProps.ap = character->currentAp;
      }
    }
    titleProps.showAp = world.combat.active;
    titleBar->setProps(titleProps);
  }

  alignMapView();
}

void LayerWorld::updateHeldMoveRepeat(int deltaTime) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& heldMove = stateManager->getState().uiState.heldMove;
  if (!heldMove.isActive) {
    return;
  }

  if (!window->getEvents().isKeyPressed(heldMove.key.sliceView())) {
    ui::setHeldMoveActive(*stateManager, false);
    return;
  }

  if (stateManager->getState().world.actionMode != model::WorldActionMode::NONE) {
    ui::setHeldMoveActive(*stateManager, false);
    return;
  }

  // Pause while town AI or combat cannot accept a move; keep hold active so repeats
  // resume.
  if (stateManager->getState().world.resolvingTownEnemyAi) {
    return;
  }

  if (stateManager->getState().world.combat.active &&
      !canPlayerIssueCombatMove(stateManager->getState())) {
    return;
  }

  model::timerStructUpdate(heldMove.initialDelay, deltaTime);
  model::timerStructUpdate(heldMove.moveDelay, deltaTime);

  if (model::timerStructIsComplete(heldMove.initialDelay) &&
      model::timerStructIsComplete(heldMove.moveDelay)) {
    model::timerStructRestart(heldMove.moveDelay);
    enqueueMapMove(*stateManager, heldMove.dx, heldMove.dy);
  }
}

void LayerWorld::update(int deltaTime) {
  Layer::update(deltaTime);

  // Hover is polled here: LayerManager has no mouse-move dispatch, and tests/game
  // only wire down/up/wheel. mouseX/Y are updated by SDL every frame.
  auto& events = window->getEvents();
  updateAimFromMouse(events.mouseX, events.mouseY);
  updateHeldMoveRepeat(deltaTime);

  auto stateManager = getStateManager();
  if (stateManager) {
    worldUpdate(window, *stateManager, deltaTime);
    state::worldProcessPendingTriggers(window, *stateManager);
    if (stateManager->getState().triggers.mapChangedThisTick) {
      syncFromState();
    }
  }

  syncWorldActionModeHighlight();
  syncActionModeCancelButton();
  syncCombatTitleBar();
}

void LayerWorld::render(int deltaTime) {
  // World is SUSPENDED while inventory/pickup is open (update does not run); still
  // refresh those action button pressed states before drawing.
  syncWorldActionModeHighlight();
  Layer::render(deltaTime);
}

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
  stateManager->enqueueAction(stateManager->getActionData(),
                              state::actions::cancelEquipRunes(),
                              0);
}

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

  subscribeAction<state::ActionEvent::UiSetCurrentPartyMember>(
      [this](auto&, auto&) { syncCurrentPartyMember(); });
  subscribeAction<state::ActionEvent::UiPickUpItem>(
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
          state::actions::setCurrentPartyMember(*partyIndex),
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
                                  state::actions::pickUpItem(item.id),
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

  const int carrying = game::inventoryWeight(*currentPartyMember, *database);
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
          state.world.activeMap,
          state.mapInstances,
          *avatar,
          game::PICKUP_PATH_RANGE,
          *database);
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
      state::actions::removeLayer(state::LayerId::PickUp),
      0);
}

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
      state::actions::removeLayer(state::LayerId::SpellCast),
      0);
}

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

  subscribeAction<state::ActionEvent::UiSetCurrentPartyMemberInventory>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::ActionEvent::UiReorderInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::ActionEvent::UiToggleEquipInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::ActionEvent::UiGiveInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::ActionEvent::UiDropInventoryItem>(
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
          state::actions::setCurrentPartyMemberInventory(*partyIndex),
          0);
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      state::actions::removeLayer(state::LayerId::Inventory),
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
      game::inventoryWeight(*inventoryPartyMember, *getDatabase());
  pageProps.weightCapacity = model::characterGetWeightCapacity(*inventoryPartyMember);
  pageProps.gold = player.gold;
  pageProps.inventory = inventoryPartyMember->inventory;
  pageProps.equipment = inventoryPartyMember->equipment;
  pageInventory->setProps(pageProps);
}

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

  subscribeAction<state::ActionEvent::UiSetCurrentPartyMemberMagic>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::ActionEvent::UiSetSpellReady>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::ActionEvent::UiCommitEquipRunes>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::ActionEvent::UiCancelEquipRunes>(
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
          state::actions::setCurrentPartyMemberMagic(*partyIndex),
          0);
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      state::actions::removeLayer(state::LayerId::Magic),
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

class LayerSpecialEvent : public Layer {
private:
  in3::SpecialEventRunner runner;
  in3::SpecialEventRunnerInterface runnerInterface;
  bmin::DynArray<ui::TextBlock> talkHistory;
  ui::KeyboardHeldScroll talkKeyboardScroll;
  bool eventFinished = false;
  bool needsSyncUi = false;
  int continuePressRemainingMs = 0;
  int choicePressRemainingMs = 0;
  std::optional<int> pendingChoiceIndex;

  void appendCurrentTalkTextToHistory();
  void appendTalkChoiceToHistory(int choiceIndex);
  void attachChoiceObservers();
  void attachModalContinueObserver();
  ui::ButtonModal* findModalContinueButton();
  ui::ButtonTextWrap* findChoiceButton(int choiceIndex);
  void beginKeyboardContinuePress();
  void beginKeyboardChoicePress(int choiceIndex);
  void closeLayer();
  void persistRunnerStorage();
  void setupTalkKeyboardScroll();
  ui::SectionScrollable* getTalkTextSection();
  ui::SectionScrollable* getTalkChoiceSection();

public:
  LayerSpecialEvent(sdl2w::Window* _window,
                    const model::GameEvent& gameEvent,
                    const bmin::Map<bmin::String, model::GameEvent>& gameEvents,
                    const bmin::Map<bmin::String, bmin::String>& initialStorage = {});
  ~LayerSpecialEvent() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void onChoiceSelected(int choiceIndex);
  void onContinue();
  void syncUi();
  void update(int deltaTime) override;
};

} // namespace layers


namespace layers {

namespace {

constexpr int TALK_CHOICE_AREA_HEIGHT = 250;
constexpr int kKeyboardPressFlashMs = 120;

ui::PageTalkChoiceProps buildTalkProps(in3::SpecialEventRunner& runner,
                                       const bmin::DynArray<ui::TextBlock>& talkHistory,
                                       int windowWidth,
                                       int windowHeight) {
  ui::PageTalkChoiceProps props;
  props.width = windowWidth;
  props.height = windowHeight;
  props.choiceAreaHeight = TALK_CHOICE_AREA_HEIGHT;
  props.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  props.portraitSpriteName = runner.gameEvent.icon;
  props.portraitScale = 1.5f;
  props.pinFromBlockIndex = static_cast<int>(talkHistory.size());
  for (const auto& block : talkHistory) {
    props.textBlocks.pushBack(block);
  }
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText;
    props.textBlocks.pushBack(block);
  }
  for (const auto& choice : runner.displayTextChoices) {
    ui::PageTalkChoiceItem item;
    item.nextId = choice.next;
    item.text = choice.text;
    item.prefixText = choice.prefix;
    item.previouslyChosen = runner.wasChoiceChosen(choice.choiceKey);
    props.choices.pushBack(item);
  }
  return props;
}

ui::PageModalEventProps
buildModalProps(in3::SpecialEventRunner& runner, int windowWidth, int windowHeight) {
  ui::PageModalEventProps props;
  // Window dims; ModalSmall default CappedCentered sizes/centers the shell.
  props.width = windowWidth;
  props.height = windowHeight;
  props.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText;
    props.textBlocks.pushBack(block);
  }
  for (const auto& choice : runner.displayTextChoices) {
    ui::PageTalkChoiceItem item;
    item.nextId = choice.next;
    item.text = choice.text;
    item.prefixText = choice.prefix;
    item.previouslyChosen = runner.wasChoiceChosen(choice.choiceKey);
    props.choices.pushBack(item);
  }
  props.showContinueButton = props.choices.empty() && !runner.displayText.empty() &&
                             !runner.getNextNodeId().empty();
  return props;
}

} // namespace

LayerSpecialEvent::LayerSpecialEvent(
    sdl2w::Window* _window,
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::GameEvent>& gameEvents,
    const bmin::Map<bmin::String, bmin::String>& initialStorage)
    : Layer(_window, state::LayerId::SpecialEvent),
      runner(initialStorage, gameEvent, gameEvents),
      runnerInterface(runner) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  if (gameEvent.eventType == model::GameEventType::TALK) {
    auto pageTalkChoice = new ui::PageTalkChoice(window);
    pageTalkChoice->setId("eventPage");
    pageTalkChoice->setPos(0, 0);
    pageTalkChoice->setScale(scale);
    pageTalkChoice->setProps(buildTalkProps(runner,
                                            talkHistory,
                                            static_cast<int>(windowWidth / scale),
                                            static_cast<int>(windowHeight / scale)));
    addUiElement(pageTalkChoice);
  } else {
    auto modalProps = buildModalProps(runner,
                                      static_cast<int>(windowWidth / scale),
                                      static_cast<int>(windowHeight / scale));
    auto pageModalEvent = new ui::PageModalEvent(window);
    pageModalEvent->setId("eventPage");
    pageModalEvent->setPos(0, 0);
    pageModalEvent->setScale(scale);
    pageModalEvent->setProps(modalProps);
    addUiElement(pageModalEvent);
  }

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  runnerInterface.startEvent();
  syncUi();
  setupTalkKeyboardScroll();

  subscribeAction<state::ActionEvent::UiSelectSpecialEventChoice>(
      [this](auto& action, auto&) { onChoiceSelected(action.getEventValue()); });
  subscribeAction<state::ActionEvent::UiContinueSpecialEvent>(
      [this](auto&, auto&) { onContinue(); });
}

void LayerSpecialEvent::appendCurrentTalkTextToHistory() {
  if (runner.displayText.empty()) {
    return;
  }
  ui::TextBlock block;
  block.text = runner.displayText + "\n\n";
  talkHistory.pushBack(block);
}

void LayerSpecialEvent::appendTalkChoiceToHistory(int choiceIndex) {
  appendCurrentTalkTextToHistory();
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }
  const auto& choice = runner.displayTextChoices[choiceIndex];
  const bmin::String label =
      choice.prefix.empty() ? choice.text : choice.prefix + " " + choice.text;
  ui::TextBlock block;
  block.text = bmin::String("> ") + label + "\n\n";
  block.fontColor = ui::Colors::DarkBlue;
  talkHistory.pushBack(block);
}

void LayerSpecialEvent::syncUi() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return;
    }
    auto [windowWidth, windowHeight] = window->getDims();
    pageTalkChoice->setProps(buildTalkProps(runner,
                                            talkHistory,
                                            static_cast<int>(windowWidth),
                                            static_cast<int>(windowHeight)));
    attachChoiceObservers();
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  auto [windowWidth, windowHeight] = window->getDims();
  auto modalProps = buildModalProps(
      runner, static_cast<int>(windowWidth), static_cast<int>(windowHeight));
  pageModalEvent->setPos(0, 0);
  pageModalEvent->setProps(modalProps);
  attachChoiceObservers();
  attachModalContinueObserver();
}

void LayerSpecialEvent::attachChoiceObservers() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return;
    }
    for (int i = 0; i < static_cast<int>(runner.displayTextChoices.size()); i++) {
      const auto choiceId = "choice" + bmin::toString(i);
      auto* choice = pageTalkChoice->getChildById(
          std::string_view(choiceId.cStr(), choiceId.size()));
      if (!choice) {
        continue;
      }
      choice->addEventObserver(new ui::ObserverSpecialEventChoice(i));
    }
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  for (int i = 0; i < static_cast<int>(runner.displayTextChoices.size()); i++) {
    const auto choiceId = "choice" + bmin::toString(i);
    auto* choice =
        pageModalEvent->getChildById(std::string_view(choiceId.cStr(), choiceId.size()));
    if (!choice) {
      continue;
    }
    choice->addEventObserver(new ui::ObserverSpecialEventChoice(i));
  }
}

void LayerSpecialEvent::attachModalContinueObserver() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    return;
  }

  auto* button = findModalContinueButton();
  if (!button) {
    return;
  }
  button->addEventObserver(new ui::ObserverSpecialEventContinue());
}

ui::ButtonModal* LayerSpecialEvent::findModalContinueButton() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    return nullptr;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return nullptr;
  }
  auto* modal = pageModalEvent->getChildById("modal");
  if (!modal) {
    return nullptr;
  }
  auto* buttonGroup = dynamic_cast<ui::ButtonGroup*>(modal->getChildById("buttonGroup"));
  if (!buttonGroup || buttonGroup->getChildren().empty()) {
    return nullptr;
  }
  return dynamic_cast<ui::ButtonModal*>(buttonGroup->getChildren()[0].get());
}

void LayerSpecialEvent::beginKeyboardContinuePress() {
  if (continuePressRemainingMs > 0 || choicePressRemainingMs > 0) {
    return;
  }

  // TALK injects a synthetic "(Continue.)" choice for non-auto-advance EXEC stops.
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    if (runner.displayTextChoices.size() == 1 &&
        runner.displayTextChoices[0].text == TRANSLATE("(Continue.)")) {
      beginKeyboardChoicePress(0);
      return;
    }
    return;
  }

  if (!runner.displayTextChoices.empty()) {
    return;
  }

  if (auto* button = findModalContinueButton()) {
    button->isActive = true;
    continuePressRemainingMs = kKeyboardPressFlashMs;
    return;
  }

  onContinue();
}

ui::ButtonTextWrap* LayerSpecialEvent::findChoiceButton(int choiceIndex) {
  const auto choiceId = "choice" + bmin::toString(choiceIndex);
  const auto choiceIdView = std::string_view(choiceId.cStr(), choiceId.size());

  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return nullptr;
    }
    return dynamic_cast<ui::ButtonTextWrap*>(pageTalkChoice->getChildById(choiceIdView));
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return nullptr;
  }
  return dynamic_cast<ui::ButtonTextWrap*>(pageModalEvent->getChildById(choiceIdView));
}

void LayerSpecialEvent::beginKeyboardChoicePress(int choiceIndex) {
  if (continuePressRemainingMs > 0 || choicePressRemainingMs > 0 || eventFinished) {
    return;
  }
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }

  pendingChoiceIndex = choiceIndex;
  choicePressRemainingMs = kKeyboardPressFlashMs;
  if (auto* button = findChoiceButton(choiceIndex)) {
    button->isActive = true;
  }
}

void LayerSpecialEvent::onChoiceSelected(int choiceIndex) {
  if (eventFinished) {
    return;
  }
  talkKeyboardScroll.stopScroll();
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    appendTalkChoiceToHistory(choiceIndex);
  }
  runnerInterface.selectChoice(choiceIndex);
  if (runner.gameEvent.eventType == model::GameEventType::TALK && runner.isAtEndNode()) {
    eventFinished = true;
    closeLayer();
    return;
  }
  if (runner.getNextNodeId().empty() && runner.displayTextChoices.empty() &&
      !runner.displayText.empty()) {
    eventFinished = true;
    closeLayer();
    return;
  }
  needsSyncUi = true;
}

void LayerSpecialEvent::onContinue() {
  if (eventFinished) {
    closeLayer();
    return;
  }

  if (!runner.displayTextChoices.empty()) {
    return;
  }

  if (runner.getNextNodeId().empty()) {
    eventFinished = true;
    closeLayer();
    return;
  }

  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    appendCurrentTalkTextToHistory();
  }
  runnerInterface.continueEvent();
  if (runner.gameEvent.eventType == model::GameEventType::TALK && runner.isAtEndNode()) {
    eventFinished = true;
    closeLayer();
    return;
  }
  if (runner.getNextNodeId().empty() && runner.displayTextChoices.empty()) {
    closeLayer();
    return;
  }
  needsSyncUi = true;
}

void LayerSpecialEvent::persistRunnerStorage() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& persisted = stateManager->getState().specialEventStorage;
  persisted = runner.storage;
  in3::clearTmpStorageKeys(persisted);
}

void LayerSpecialEvent::closeLayer() {
  talkKeyboardScroll.stopScroll();
  choicePressRemainingMs = 0;
  pendingChoiceIndex.reset();
  continuePressRemainingMs = 0;
  persistRunnerStorage();
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      state::actions::removeLayer(state::LayerId::SpecialEvent),
      0);
}

ui::SectionScrollable* LayerSpecialEvent::getTalkTextSection() {
  auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
  if (!pageTalkChoice) {
    return nullptr;
  }
  return dynamic_cast<ui::SectionScrollable*>(
      pageTalkChoice->getChildById("textSection"));
}

ui::SectionScrollable* LayerSpecialEvent::getTalkChoiceSection() {
  auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
  if (!pageTalkChoice) {
    return nullptr;
  }
  return dynamic_cast<ui::SectionScrollable*>(
      pageTalkChoice->getChildById("choiceSection"));
}

void LayerSpecialEvent::setupTalkKeyboardScroll() {
  talkKeyboardScroll.clearBindings();
  if (runner.gameEvent.eventType != model::GameEventType::TALK) {
    return;
  }

  // Left/right (and numpad) scroll dialogue history; up/down scroll choices.
  // Section getters resolve live pointers so rebuilds in syncUi stay safe.
  auto textSection = [this]() { return getTalkTextSection(); };
  auto choiceSection = [this]() { return getTalkChoiceSection(); };

  talkKeyboardScroll.bindSectionKey("Left", textSection, ui::HeldScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Keypad 4", textSection, ui::HeldScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Right", textSection, ui::HeldScrollDirection::Down);
  talkKeyboardScroll.bindSectionKey("Keypad 6", textSection, ui::HeldScrollDirection::Down);
  talkKeyboardScroll.bindSectionKey("Up", choiceSection, ui::HeldScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Keypad 8", choiceSection, ui::HeldScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Down", choiceSection, ui::HeldScrollDirection::Down);
  talkKeyboardScroll.bindSectionKey("Keypad 2", choiceSection, ui::HeldScrollDirection::Down);
}

void LayerSpecialEvent::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }

  // Talk must be exited via choices / the modal close control — not Escape.
  if (key == "Escape") {
    if (runner.gameEvent.eventType != model::GameEventType::TALK) {
      closeLayer();
    }
    return;
  }

  if (talkKeyboardScroll.onKeyDown(key)) {
    return;
  }

  if (key == "Return" || key == "Keypad Enter" || key == "space") {
    talkKeyboardScroll.stopScroll();
    beginKeyboardContinuePress();
    return;
  }

  if (key.size() == 1 && key[0] >= '1' && key[0] <= '9') {
    talkKeyboardScroll.stopScroll();
    const auto choiceIndex = static_cast<int>(key[0] - '1');
    beginKeyboardChoicePress(choiceIndex);
  }
}

void LayerSpecialEvent::onKeyUp(std::string_view key, int /*keyCode*/) {
  talkKeyboardScroll.onKeyUp(key);
}

void LayerSpecialEvent::update(int deltaTime) {
  Layer::update(deltaTime);
  talkKeyboardScroll.update(deltaTime, window);

  if (choicePressRemainingMs > 0) {
    choicePressRemainingMs -= deltaTime;
    if (choicePressRemainingMs <= 0) {
      choicePressRemainingMs = 0;
      const int choiceIndex = pendingChoiceIndex.value_or(-1);
      pendingChoiceIndex.reset();
      if (auto* button = findChoiceButton(choiceIndex)) {
        button->isActive = false;
      }
      if (choiceIndex >= 0) {
        onChoiceSelected(choiceIndex);
      }
    }
  }

  if (continuePressRemainingMs > 0) {
    continuePressRemainingMs -= deltaTime;
    if (continuePressRemainingMs <= 0) {
      continuePressRemainingMs = 0;
      if (auto* button = findModalContinueButton()) {
        button->isActive = false;
      }
      onContinue();
    }
  }

  if (needsSyncUi) {
    needsSyncUi = false;
    syncUi();
  }
}

Layer* createWorldLayer(sdl2w::Window* window, float mapScale) {
  auto* layer = new LayerWorld(window);
  layer->setMapScale(mapScale);
  return layer;
}

Layer* createInventoryLayer(sdl2w::Window* window) {
  return new LayerInventory(window);
}

Layer* createPickUpLayer(sdl2w::Window* window) {
  return new LayerPickUp(window);
}

} // namespace layers
