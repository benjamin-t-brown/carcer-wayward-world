module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <optional>
#include <functional>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <algorithm>
#include <vector>

export module carcer.ui.screens;
export import bmin.containers;
export import carcer.data;
export import carcer.lib.StringUtil;
export import carcer.model;
export import carcer.state;
export import carcer.ui.core;
export import carcer.ui.widgets;
import bmin.string_interop;
import carcer.actions;
import sdl2w;
#include "macros.h"

export {

// --- from ui/helpers/modalLayoutFit.h ---
namespace ui {


// How ModalStandard / ModalSmall map width/height props to the final rect.
// CappedCentered (default): treat width/height as window dims, then cap + center.
// FullBleed: use width/height as the modal size as-is (no cap/center).
enum class LayoutFit { CappedCentered, FullBleed };

// Landscape max size class. Portrait still uses near-full margins for both.
enum class ModalSizeClass { Standard, Small };

struct LayoutRect {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

// Portrait: near-full with small margins.
// Landscape Standard: ~500×min(500, h−50). Small: ~400×min(420, h−50).
LayoutRect computeCappedCenteredRect(int windowW,
                                     int windowH,
                                     ModalSizeClass sizeClass = ModalSizeClass::Standard);

// After a CappedCentered modal->setProps, sync the host page/minipage style to the
// fitted rect so getPos()+getDims() (often child[0] size) align with drawn controls.
// Call while host style still holds window width/height. Pass the same sizeClass
// the modal used.
void syncHostStyleToCappedCentered(BaseStyle& style,
                                   ModalSizeClass sizeClass = ModalSizeClass::Standard);

} // namespace ui

// --- from ui/helpers/keyboardShortcuts.h ---
namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key, model::TurnMode turnMode);

struct MoveDelta {
  int dx = 0;
  int dy = 0;
};

std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key);

bool isCancelActionKey(std::string_view key);

bool isConfirmActionKey(std::string_view key);

bool isCombatWaitKey(std::string_view key);

/** `m` / `M` — open combat spell-cast list (combat-only at call site). */
bool isOpenSpellCastKey(std::string_view key);

/** `r` / `R` — open magic setup (LayerMagic); always, combat or not. */
bool isOpenMagicSetupKey(std::string_view key);

/** Keys "1"-"6" → party index 0-5. */
std::optional<int> getPartyMemberIndexFromKey(std::string_view key);

/** Keys "a"-"z" / "A"-"Z" → pick-up list index 0-25. */
std::optional<int> getPickUpItemIndexFromKey(std::string_view key);

} // namespace ui

// --- from ui/helpers/worldActions.h ---
namespace ui {

void setHeldMoveActive(state::StateManager& stateManager, bool isActive);
void cancelCurrentWorldActionMode(state::StateManager& stateManager);
void showMagicSetupLayer(state::StateManager& stateManager, sdl2w::Window* window);
void showSpellCastLayer(state::StateManager& stateManager, sdl2w::Window* window);
void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType,
                         sdl2w::Window* window = nullptr);

} // namespace ui

} // export

export {

// --- from ui/observers/ObserverRemoveLayer.hpp ---
namespace ui {
class ObserverRemoveLayer : public ui::UiEventObserver,
                            public state::StateManagerInterface {
  bmin::String layerId;

public:
  ObserverRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  ObserverRemoveLayer(std::string_view _layerId) : layerId(strutil::fromStringView(_layerId)) {}
  explicit ObserverRemoveLayer(state::LayerId id)
      : layerId(strutil::fromStringView(state::layerIdString(id))) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverRemoveLayer::onClick " << layerId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || layerId.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::removeLayer(layerId), 0);
  }
};
} // namespace ui

} // export

export {

// Choice / continue clicks in a special event go through the action bus, not
// a back-pointer into layers::LayerSpecialEvent — UI observers don't reach
// into a concrete Layer. LayerSpecialEvent subscribes to react.
namespace ui {

class ObserverSpecialEventChoice : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  int choiceIndex;

public:
  explicit ObserverSpecialEventChoice(int _choiceIndex) : choiceIndex(_choiceIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::selectSpecialEventChoice(choiceIndex),
        0);
  }
};

class ObserverSpecialEventContinue : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
public:
  void onClick(int mouseX, int mouseY, int button) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::continueSpecialEvent(), 0);
  }
};

} // namespace ui

} // export

export {

// --- from ui/KeyboardHeldScroll.h ---
namespace ui {

enum class HeldScrollDirection { Up, Down };

// Hold-to-repeat keyboard scrolling for any SectionScrollable (or custom action).
// Wire from a Layer/Page: onKeyDown / onKeyUp / update. Timing matches world movement
// (first step immediately, 300ms pause, then every 50ms).
class KeyboardHeldScroll {
private:
  struct Binding {
    bmin::String key;
    std::function<void()> action;
  };

  struct Held {
    bool isActive = false;
    std::function<void()> action;
    bmin::String key;
    int dx = 0;
    int dy = 0;
    model::TimerStruct initialDelay = model::TimerStruct(300);
    model::TimerStruct moveDelay = model::TimerStruct(50);
  };

  const Binding* findBinding(std::string_view key) const;
  void applyHeldAction();

  bmin::DynArray<Binding> bindings;
  Held held;

public:
  void clearBindings();
  void stopScroll();

  // Bind a key to an arbitrary scroll/step action.
  void bindKey(std::string_view key, std::function<void()> action);

  // Bind a key to scrollUp/scrollDown on a section resolved each step (safe across UI
  // rebuilds).
  void bindSectionKey(std::string_view key,
                      std::function<SectionScrollable*()> sectionGetter,
                      HeldScrollDirection direction);

  // Returns true if the key matched a binding and was handled.
  bool onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void update(int deltaTime, sdl2w::Window* window);
  bool isHolding() const { return held.isActive; }
};

} // namespace ui

} // export

export {

// --- from ui/layouts/ModalSmall.h ---
// IWYU pragma: keep

namespace ui {

// ModalSmall layout properties
struct ModalSmallProps {
  // For LayoutFit::CappedCentered (default), width/height are window dims.
  // For LayoutFit::FullBleed, width/height are the modal size as-is.
  int width = 0;
  int height = 0;
  LayoutFit layoutFit = LayoutFit::CappedCentered;
  SDL_Color backgroundColor = Colors::White;
  bmin::String iconSprite = "";
  /** Scales headerHeight (base 80), icon well (base 64), and the header icon sprite. */
  float iconScale = 1.f;
  bool enableCloseButton = true;
};

// ModalSmall layout - renders a small modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalSmall : public UiElement {
private:
  ModalSmallProps props;

  int getScaledButtonsAreaHeight();

public:
  static const int BUTTONS_AREA_HEIGHT = 50;

  ModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalSmall() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalSmallProps& _props);
  ModalSmallProps& getProps();
  const ModalSmallProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();
  const std::pair<int, int> getButtonsDims();
  const std::pair<int, int> getButtonsLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/layouts/ModalStandard.h ---
// IWYU pragma: keep

namespace ui {

// ModalStandard layout properties
struct ModalStandardProps {
  // For LayoutFit::CappedCentered (default), width/height are window dims.
  // For LayoutFit::FullBleed, width/height are the modal size as-is.
  int width = 0;
  int height = 0;
  LayoutFit layoutFit = LayoutFit::CappedCentered;
  SDL_Color contentBackgroundColor = Colors::White;
  bmin::String decorationSprite = "";
  bmin::String iconSprite;
  // Scales headerHeight (base 80) and the portrait that fits inside the top-left
  // OutsetRectangle well (headerHeight - 2 * outset border).
  float portraitScale = 1.f;
};

// ModalStandard layout - renders a modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalStandard : public UiElement {
private:
  ModalStandardProps props;

public:
  ModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalStandard() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalStandardProps& _props);
  ModalStandardProps& getProps();
  const ModalStandardProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();

  const std::pair<int, int> getSubTitleDims();
  const std::pair<int, int> getSubTitleLocation();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/layouts/InGameLayout.h ---
// IWYU pragma: keep

namespace ui {


enum class InGameBorderType { Wide, Narrow };

// InGameLayout layout properties
struct InGameLayoutProps {
  int width = 0;
  int height = 0;
  bmin::DynArray<state::WorldActionType> worldActionTypes;
  bmin::DynArray<ChCompactInfoProps> partyMembers;
  int selectedPartyMemberIndex = 0;
  float actionButtonScale = 1.f;
  InGameBorderType borderType = InGameBorderType::Wide;
};

// InGameLayout layout - renders an in-game layout with background, border, title,
// subtitle, and children Uses Position, Size, Scale from BaseStyle
class InGameLayout : public UiElement {
private:
  InGameLayoutProps props;

  void applyTitleLayout(UiElement* titleElement, BorderInGame* border);
  void buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                          int actionButtonsQuadWidth,
                          int actionButtonsQuadHeight,
                          Quad* actionButtonsQuad);
  void buildChList(const std::pair<int, int>& chListLocation);

public:
  InGameLayout(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameLayout() override = default;

  void setProps(const InGameLayoutProps& _props);
  InGameLayoutProps& getProps();
  const InGameLayoutProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  const std::pair<int, int> getWorldDims();
  const std::pair<int, int> getWorldLocation();
  const std::pair<int, int> getChListLocation();

  // Modal-style X + mode label in the bottom-left of the map area.
  void setActionModeCancelVisible(bool visible, const bmin::String& modeLabel = {});

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverCancelWorldActionMode.hpp ---
namespace ui {

class ObserverCancelWorldActionMode : public UiEventObserver {
  state::StateManager* stateManager;

public:
  explicit ObserverCancelWorldActionMode(state::StateManager* _stateManager)
      : stateManager(_stateManager) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverWorldAction.hpp ---
namespace ui {

class ObserverWorldAction : public UiEventObserver {
  state::StateManager* stateManager;
  state::WorldActionType worldActionType;
  sdl2w::Window* window;

public:
  ObserverWorldAction(state::StateManager* _stateManager,
                      state::WorldActionType _worldActionType,
                      sdl2w::Window* _window)
      : stateManager(_stateManager),
        worldActionType(_worldActionType),
        window(_window) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetSelectedPartyMemberId.hpp ---
namespace ui {

class ObserverSetSelectedPartyMemberId : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  bmin::String partyMemberId;

public:
  explicit ObserverSetSelectedPartyMemberId(bmin::String _partyMemberId)
      : partyMemberId(std::move(_partyMemberId)) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

export {

// --- from ui/popups/PopupDropConfirm.h ---
namespace ui {

struct PopupDropConfirmProps {
  bmin::String characterPlayerId;
  bmin::String itemId;
  bmin::String itemLabel;
};

class PopupDropConfirm : public UiElement {
  PopupDropConfirmProps props;

public:
  PopupDropConfirm(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PopupDropConfirm() override;

  void setProps(const PopupDropConfirmProps& _props);
  PopupDropConfirmProps& getProps();
  const PopupDropConfirmProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverDropInventoryItem.hpp ---
namespace ui {

class ObserverDropInventoryItem : public ui::UiEventObserver,
                                  public state::StateManagerInterface {
  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverDropInventoryItem(const bmin::String& _characterPlayerId, const bmin::String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

export {

// --- from ui/popups/PopupGive.h ---
namespace ui {


struct PopupGivePartyMember {
  bmin::String characterPlayerId;
  bmin::String label;
  bmin::String spriteName;
};

struct PopupGiveProps {
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;
  bmin::String itemLabel;
  int maxQuantity = 1;
  int selectedQuantity = 1;
  bool showQuantitySlider = true;
  bmin::DynArray<PopupGivePartyMember> partyMembers;
};

class PopupGive : public UiElement {
  PopupGiveProps props;
  HorizontalSlider* quantitySlider = nullptr;

public:
  PopupGive(sdl2w::Window* _window, UiElement* _parent = nullptr);

  void setProps(const PopupGiveProps& _props);
  PopupGiveProps& getProps();
  const PopupGiveProps& getProps() const;

  int getSelectedQuantity() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverGiveInventoryItem.hpp ---
namespace ui {

class ObserverGiveInventoryItem : public ui::UiEventObserver,
                                  public state::StateManagerInterface {
  bmin::String toCharacterPlayerId;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;
  PopupGive* popupGive;

public:
  ObserverGiveInventoryItem(const bmin::String& _toCharacterPlayerId,
                            const bmin::String& _fromCharacterPlayerId,
                            const bmin::String& _itemId,
                            PopupGive* _popupGive)
      : toCharacterPlayerId(_toCharacterPlayerId),
        fromCharacterPlayerId(_fromCharacterPlayerId),
        itemId(_itemId),
        popupGive(_popupGive) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

export {

// --- from ui/popups/PopupInventoryItem.h ---
namespace ui {

enum PopupOrientation { NARROW, WIDE };

struct PopupInventoryItemProps {
  bmin::String characterPlayerId;
  model::ItemInstance item;
  bmin::String label;
  bmin::String description;
  bmin::String spriteName;
  int weight = 0;
  int value = 0;
  bool usable = false;
  bool equippable = false;
  PopupOrientation orientation = WIDE;
};

class PopupInventoryItem : public UiElement {
  PopupInventoryItemProps props;

public:
  PopupInventoryItem(sdl2w::Window* _window,
                     UiElement* _parent,
                     PopupOrientation _orientation = WIDE);

  void setProps(const PopupInventoryItemProps& _props);
  PopupInventoryItemProps& getProps();
  const PopupInventoryItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerDropContext.hpp ---
namespace ui {

class ObserverShowLayerDropContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverShowLayerDropContext(sdl2w::Window* _window,
                               const bmin::String& _characterPlayerId,
                               const bmin::String& _itemId)
      : window(_window),
        characterPlayerId(_characterPlayerId),
        itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerGiveContext.hpp ---
namespace ui {

class ObserverShowLayerGiveContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

public:
  ObserverShowLayerGiveContext(sdl2w::Window* _window,
                               const bmin::String& _fromCharacterPlayerId,
                               const bmin::String& _itemId)
      : window(_window),
        fromCharacterPlayerId(_fromCharacterPlayerId),
        itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

export {

// --- from ui/popups/PopupPickupItem.h ---
namespace ui {

struct PopupPickupItemProps {
  bmin::String spriteName;
  bmin::String label;
  bmin::String description;
  int weight = 0;
  int value = 0;
  PopupOrientation orientation = WIDE;
};

// PopupPickupItem - shows info about an item that can be picked up
class PopupPickupItem : public UiElement {
  PopupPickupItemProps props;
  bmin::String closeLayerId;

public:
  PopupPickupItem(sdl2w::Window* _window,
                  bmin::String _closeLayerId,
                  PopupOrientation _orientation = WIDE);

  void setProps(const PopupPickupItemProps& _props);
  PopupPickupItemProps& getProps();
  const PopupPickupItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/popups/PopupSpellInfo.h ---
namespace ui {

struct PopupSpellInfoRuneReq {
  bmin::String iconSprite;
  int count = 1;
};

struct PopupSpellInfoProps {
  bmin::String label;
  bmin::String description;
  bmin::String spriteName;
  int manaCost = 0;
  bmin::DynArray<PopupSpellInfoRuneReq> requiredRunes;
  PopupOrientation orientation = WIDE;
};

// Spell info popup: name, mana cost, required runes, description; close only.
class PopupSpellInfo : public UiElement {
  PopupSpellInfoProps props;

public:
  PopupSpellInfo(sdl2w::Window* _window,
                 UiElement* _parent = nullptr,
                 PopupOrientation _orientation = WIDE);

  void setProps(const PopupSpellInfoProps& _props);
  PopupSpellInfoProps& getProps();
  const PopupSpellInfoProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/minipages/MinipageCharacterSheet.h ---
namespace ui {

struct MinipageCharacterSheetProps {
  int width = 0;
  int height = 0;
};

// MinipageCharacterSheet - renders a small character sheet shell using ModalSmall.
class MinipageCharacterSheet : public UiElement {
private:
  MinipageCharacterSheetProps props;

public:
  MinipageCharacterSheet(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageCharacterSheet() override = default;

  void setProps(const MinipageCharacterSheetProps& _props);
  MinipageCharacterSheetProps& getProps();
  const MinipageCharacterSheetProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/minipages/MinipageEquipRunes.h ---
namespace ui {

struct MinipageEquipRunesSlot {
  bool filled = false;
  bmin::String iconSprite;
};

struct MinipageEquipRunesRow {
  model::RuneType type = model::RuneType::HEAT;
  bmin::String iconSprite;
  int availableCount = 0;
  int equippedCount = 0;
};

struct MinipageEquipRunesProps {
  int width = 0;
  int height = 0;
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::DynArray<MinipageEquipRunesSlot> runeSlots;
  bmin::DynArray<MinipageEquipRunesRow> runeRows;
};

/** ModalSmall editor: equipped strip + available rune +/- list; Okay commits, X cancels. */
class MinipageEquipRunes : public UiElement {
private:
  MinipageEquipRunesProps props;

  static constexpr int contentPadding = 8;
  static constexpr int nameRowHeight = 28;
  static constexpr int runeSlotSize = 24;
  static constexpr int runeSlotGap = 4;
  static constexpr int runeSlotIconSize = 24;
  static constexpr float runeSlotIconScale = 1.f;
  static constexpr int equippedRowHeight = 40;
  static constexpr int gridCols = 2;
  // Match PageCharacter +/- ButtonIcon size (32).
  static constexpr int adjustButtonSize = 32;
  static constexpr int cellHeight = adjustButtonSize;
  static constexpr int cellGapX = 8;
  static constexpr int cellGapY = 4;
  static constexpr int cellInnerGap = 2;
  static constexpr int countTextWidth = 20;
  static constexpr int footerButtonWidth = 100;
  static constexpr int modalBorderWidth = 2;
  static constexpr int modalHeaderHeight = 80;

  int runeCellContentWidth() const;
  int contentInnerWidth() const;
  int contentInnerHeight() const;
  int modalWidth() const;
  int modalHeight() const;

  void addEquippedSlots(UiElement* parent, int x, int y, int width);
  void addRuneGrid(UiElement* parent, int x, int y, int width);

public:
  MinipageEquipRunes(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageEquipRunes() override = default;

  void setProps(const MinipageEquipRunesProps& _props);
  MinipageEquipRunesProps& getProps();
  const MinipageEquipRunesProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverAdjustEquippedRune.hpp ---
namespace ui {

class ObserverAdjustEquippedRune : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  bmin::String characterPlayerId;
  model::RuneType runeType = model::RuneType::HEAT;
  int delta = 0;

public:
  ObserverAdjustEquippedRune(const bmin::String& _characterPlayerId,
                             model::RuneType _runeType,
                             int _delta)
      : characterPlayerId(_characterPlayerId), runeType(_runeType), delta(_delta) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverCancelEquipRunes.hpp ---
namespace ui {

class ObserverCancelEquipRunes : public ui::UiEventObserver,
                                 public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverCommitEquipRunes.hpp ---
namespace ui {

class ObserverCommitEquipRunes : public ui::UiEventObserver,
                                 public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

export {

// --- from ui/minipages/MinipageEvent.h ---
namespace ui {

struct MinipageEventProps {
  int width = 0;
  int height = 0;
};

// MinipageEvent - renders a small event shell using ModalSmall.
class MinipageEvent : public UiElement {
private:
  MinipageEventProps props;

public:
  MinipageEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageEvent() override = default;

  void setProps(const MinipageEventProps& _props);
  MinipageEventProps& getProps();
  const MinipageEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/minipages/MinipagePickUp.h ---
namespace ui {

struct MinipagePickUpProps {
  int width = 0;
  int height = 0;
  bmin::String titleText = TRANSLATE("Pick Up");
  bmin::String statusText;
  bmin::String weightText = "Carrying 0/100";
  int partyMemberIndex = 0;
  bmin::DynArray<bmin::String> partyMemberSprites;
  bmin::DynArray<model::ItemInstance> nearbyItems;
  bmin::String doneButtonRemoveLayerId;
};

// MinipagePickUp - renders the pickup minipage with ModalSmall layout containing
// a scrollable list of items that can be picked up.
class MinipagePickUp : public UiElement, public state::DatabaseInterface {
private:
  MinipagePickUpProps props;

public:
  MinipagePickUp(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipagePickUp() override = default;

  void setProps(const MinipagePickUpProps& _props);
  MinipagePickUpProps& getProps();
  const MinipagePickUpProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/minipages/MinipageSpellCast.h ---
namespace ui {

struct MinipageSpellCastSpell {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  // GCC BMI: DynArray nested in DynArray under UiElement corrupts GCM.
  std::vector<bmin::String> requiredRuneSprites;
};

struct MinipageSpellCastProps {
  bmin::String casterId;
  int width = 0;
  int height = 0;
  bmin::String titleText = TRANSLATE("Cast Spell");
  bmin::String statusText;
  bmin::DynArray<MinipageSpellCastSpell> spells;
  bmin::String doneButtonRemoveLayerId;
};

/** ModalSmall combat cast list: known spells; row click validates then aims. */
class MinipageSpellCast : public UiElement {
private:
  MinipageSpellCastProps props;

public:
  MinipageSpellCast(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageSpellCast() override = default;

  void setProps(const MinipageSpellCastProps& _props);
  MinipageSpellCastProps& getProps();
  const MinipageSpellCastProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/pages/PageCharacter.h ---
namespace ui {


struct PageCharacterProps {
  int width = 0;
  int height = 0;
  model::CharacterPlayer* characterPlayer = nullptr;
};

struct PageCharacterStatRowEntry {
  bmin::String label;
  bmin::String helpDescription;
  int value = 0;
  bmin::String valueText;
};

struct PageCharacterStatRowSectionArgs {
  bmin::String title;
  bmin::DynArray<PageCharacterStatRowEntry> rows;
  int width  = 0;
  int y = 0;
  bool showModButtons = false;
  bool buttonMinusDisabled = false;
  bool buttonPlusDisabled = false;
};

// PageCharacter - character stats and level-up UI inside a modal.
class PageCharacter : public UiElement, public state::DatabaseInterface {
private:
  PageCharacterProps props;

  void addDerivedStatSections(SectionScrollable* scrollable,
                              int sectionContentW,
                              int& yAgg,
                              const model::CharacterDerivedStats& derivedStats);

public:
  PageCharacter(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageCharacter() override = default;

  void setProps(const PageCharacterProps& _props);
  PageCharacterProps& getProps();
  const PageCharacterProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  UiElement* buildStatSection(const PageCharacterStatRowSectionArgs& section);

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerPopupText.hpp ---
namespace ui {

class ObserverShowLayerPopupText : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String title;
  bmin::String helpText;

public:
  ObserverShowLayerPopupText(sdl2w::Window* _window,
                             bmin::String _title,
                             bmin::String _helpText)
      : window(_window), title(std::move(_title)), helpText(std::move(_helpText)) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

export {

// --- from ui/pages/PageInventory.h ---
namespace ui {

struct PageInventoryPartyMember {
  bmin::String spriteName;
};

struct PageInventoryProps {
  int width = 0;
  int height = 0;
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::String characterPlayerSprite;
  // Passed through to ModalStandard; scales headerHeight + portrait together.
  float portraitScale = 1.f;
  int partyMemberInventoryIndex = 0;
  bmin::DynArray<PageInventoryPartyMember> partyMembers;
  int weightCarrying = 0;
  int weightCapacity = 0;
  int gold = 0;
  bmin::DynArray<model::CharacterInventoryItem> inventory;
  model::CharacterPlayerEquipment equipment;
};

class PageInventory : public UiElement, public state::DatabaseInterface {
private:
  PageInventoryProps props;

  void populateInventoryProps(bmin::DynArray<ListInventoryPropsItem>& listProps);

public:
  PageInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageInventory() override = default;

  void setProps(const PageInventoryProps& _props);
  PageInventoryProps& getProps();
  const PageInventoryProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/pages/PageMagicSetup.h ---
namespace ui {


struct PageMagicSetupPartyMember {
  bmin::String spriteName;
};

struct PageMagicSetupSpellEntry {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  /** Shown in the label as "[cost]" before "(r)" when ready. */
  int manaCost = 0;
  /** Equipped runes meet requiredRunes — shown as "(r)" on the label. */
  bool ready = false;
  /** Required rune icons expanded by count (right-justified on the spell row). */
  // GCC BMI: DynArray nested in DynArray under UiElement corrupts GCM.
  std::vector<bmin::String> requiredRuneSprites;
};

struct PageMagicSetupRuneSlot {
  bool filled = false;
  bmin::String iconSprite;
};

struct PageMagicSetupElementCount {
  bmin::String iconSprite;
  int count = 0;
};

struct PageMagicSetupProps {
  int width = 0;
  int height = 0;
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::String characterPlayerSprite;
  // Passed through to ModalStandard; scales headerHeight + portrait together.
  float portraitScale = 1.f;
  int partyMemberMagicIndex = 0;
  bmin::DynArray<PageMagicSetupPartyMember> partyMembers;
  bmin::DynArray<PageMagicSetupRuneSlot> runeSlots;
  int selectedRuneSlotIndex = -1;
  bmin::DynArray<PageMagicSetupElementCount> elementCounts;
  /** Single spell list (known spells); ready ones get "(r)" in the UI. */
  bmin::DynArray<PageMagicSetupSpellEntry> spells;
};

// PageMagicSetup - renders the magic page with ModalStandard layout.
class PageMagicSetup : public UiElement {
private:
  PageMagicSetupProps props;

  static constexpr int statusStripPadding = 8;
  static constexpr int manaSlotSize = 24;
  static constexpr int manaSlotGap = 4;
  // Rune sprites are 24x24 (Sprites,runes); drawSprite uses native size × scale.
  static constexpr int manaSlotIconSize = 24;
  static constexpr float manaSlotIconScale = 1.f;
  static constexpr int editRunesButtonWidth = 64;
  static constexpr int editRunesButtonHeight = 32;
  static constexpr int editRunesButtonGap = 8;
  static constexpr int elementGridCols = 4;
  static constexpr int elementCellWidth = 32;
  // Count label (~TEXT_SIZE_14) + 24px icon; was 32 and clipped the next row's numbers.
  static constexpr int elementCellHeight = 48;
  // Same 24x24 runes sprites as equipped slots.
  static constexpr int elementIconSize = 24;
  static constexpr float elementIconScale = 1.f;
  // Two available-rune rows (wide: side-by-side strip; narrow: stacked under equipped).
  static constexpr int availableRunesGridHeight = elementCellHeight * 2;
  // Matches modalLayoutFit portrait threshold (tall phone / portrait tablet).
  static constexpr float narrowAspectMin = 1.25f;
  static constexpr int spellPanelScrollBarWidth = 32;
  static constexpr int spellPanelHeaderPadding = 4;

  enum class StatusAlign { Left, Center, Right };

  bool isNarrowLayout() const;
  int equippedRowHeight() const;
  int statusAreaHeight(bool narrow) const;

  void addPortraitBackground(ModalStandard* modal);
  void addPartyMemberSelector(ModalStandard* modal);
  void addRuneSlotRow(ModalStandard* modal,
                      int contentX,
                      int contentW,
                      int rowY,
                      int rowHeight,
                      StatusAlign align);
  void addElementCountGrid(ModalStandard* modal,
                           int contentX,
                           int contentW,
                           int gridTopY,
                           StatusAlign align);
  void addSpellsPanel(int x, int y, int width, int height);
  ListMagicSpellsProps makeSpellListProps(int width) const;

public:
  PageMagicSetup(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageMagicSetup() override = default;

  void setProps(const PageMagicSetupProps& _props);
  PageMagicSetupProps& getProps();
  const PageMagicSetupProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerEquipRunes.hpp ---
namespace ui {

class ObserverShowLayerEquipRunes : public ui::UiEventObserver,
                                    public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String characterPlayerId;

public:
  ObserverShowLayerEquipRunes(sdl2w::Window* _window,
                              const bmin::String& _characterPlayerId)
      : window(_window), characterPlayerId(_characterPlayerId) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetSpellReady.hpp ---
namespace ui {

class ObserverSetSpellReady : public ui::UiEventObserver,
                              public state::StateManagerInterface {
  bmin::String characterPlayerId;
  bmin::String spellName;
  bool ready = true;

public:
  ObserverSetSpellReady(const bmin::String& _characterPlayerId,
                        const bmin::String& _spellName,
                        bool _ready)
      : characterPlayerId(_characterPlayerId), spellName(_spellName), ready(_ready) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverToggleManaSlotRune.hpp ---
namespace ui {

class ObserverToggleManaSlotRune : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  bmin::String characterPlayerId;
  size_t slotIndex = 0;

public:
  ObserverToggleManaSlotRune(const bmin::String& _characterPlayerId, size_t _slotIndex)
      : characterPlayerId(_characterPlayerId), slotIndex(_slotIndex) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

export {

// --- from ui/pages/PageTalkChoice.h ---
namespace ui {

struct PageTalkChoiceItem {
  bmin::String nextId;
  bmin::String text;
  bmin::String prefixText;
  bool previouslyChosen = false;
};
struct PageTalkChoiceProps {
  int width = 0;
  int height = 0;
  bmin::String title;
  bmin::String portraitSpriteName;
  // Passed through to ModalStandard; scales headerHeight + portrait together.
  float portraitScale = 1.f;
  int choiceAreaHeight = 100;
  bmin::DynArray<PageTalkChoiceItem> choices;
  bmin::DynArray<TextBlock> textBlocks;
  // Blocks [pinFromBlockIndex..) are the latest dialogue; scrolled to the top of the
  // text viewport with dynamic bottom padding so older history stays above.
  int pinFromBlockIndex = 0;
  // Multiplier for blank lines from `\n\n` in dialogue paragraphs (see TextParagraph).
  float blankLineHeightScale = 0.35f;
  // Multiplier for content line box height in dialogue paragraphs (see TextParagraph).
  float lineHeightScale = 0.85f;
};

class PageTalkChoice : public UiElement {
private:
  PageTalkChoiceProps props;
  const int SEP_BORDER_HEIGHT = 10;

  // Split dialogue on "..." boundaries: outside quotes → outsideColor (narrative);
  // inside quotes → Charcoal (spoken dialogue), unless the source already set a fontColor.
  static bmin::DynArray<TextBlock>
  colorizeDialogueByQuotes(const bmin::DynArray<TextBlock>& blocks,
                           SDL_Color outsideColor);

public:
  PageTalkChoice(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageTalkChoice() override = default;

  // Setters and getters for page-specific properties
  void setProps(const PageTalkChoiceProps& _props);
  PageTalkChoiceProps& getProps();
  const PageTalkChoiceProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/pages/PageModalEvent.h ---
namespace ui {

struct PageModalEventProps {
  int width = 500;
  int height = 400;
  bmin::String title;
  bmin::DynArray<PageTalkChoiceItem> choices;
  bmin::DynArray<TextBlock> textBlocks;
  bool showContinueButton = false;
};

// Centered small-modal page for MODAL special events (ModalSmall layout).
class PageModalEvent : public UiElement {
private:
  PageModalEventProps props;

public:
  PageModalEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageModalEvent() override = default;

  void setProps(const PageModalEventProps& _props);
  PageModalEventProps& getProps();
  const PageModalEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export
