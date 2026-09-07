module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <string_view>
#include <string>
#include <cmath>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <limits>

export module carcer.ui.widgets.composites;
export import carcer.ui.widgets.foundation;
export import bmin.containers;
export import carcer.model;
export import carcer.state;
export import carcer.ui.core;
import bmin.string_interop;
import carcer.actions;
import carcer.data;
import carcer.game.map.TileFields;
import carcer.game.map;
import sdl2w;
#include "macros.h"

export {

// --- from ui/components/lists/ListInventory.h ---
namespace ui {

struct ListInventoryPropsItem {
  bmin::String itemId;
  bmin::String itemName;
  bmin::String itemLabel;
  bmin::String itemSprite;
  bool isEquippable = false;
  bool isEquipped = false;
  bmin::String equippedSlotAbbrev;
  bool isStackable = false;
  int quantity = 1;
};
struct ListInventoryProps {
  bmin::String characterPlayerId;
  bmin::DynArray<ListInventoryPropsItem> items;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

// ListInventory - renders a vertical list of inventory items
class ListInventory : public UiElement {
private:
  ListInventoryProps props;

  const int contextBtnSize = 32;
  const int iconSpriteSize = 16;
  const int indexColumnWidth = 28;
  const int indexPaddingLeft = 4;
  const int reorderBtnHeight = 28;
  const int reorderBtnWidth = 14;
  const int reorderBtnGap = 2;
  const int reorderColumnGap = 2;

  UiElement* createItemElement(const ListInventoryPropsItem& item, int index);

public:
  ListInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListInventory() override = default;

  void setProps(const ListInventoryProps& props);
  const ListInventoryProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverInventorySelectItem.hpp ---
namespace ui {
class ObserverInventorySelectItem : public ui::UiEventObserver,
                                    public state::StateManagerInterface {

  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverInventorySelectItem(const bmin::String& _characterPlayerId,
                              const bmin::String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};
} // namespace ui

// --- from ui/observers/ObserverReorderInventoryItem.hpp ---
namespace ui {

class ObserverReorderInventoryItem : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  bmin::String characterPlayerId;
  int inventoryIndex;
  int direction;

public:
  ObserverReorderInventoryItem(const bmin::String& _characterPlayerId,
                               int _inventoryIndex,
                               int _direction)
      : characterPlayerId(_characterPlayerId),
        inventoryIndex(_inventoryIndex),
        direction(_direction) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerInventoryContext.hpp ---
namespace ui {
class ObserverShowLayerInventoryContext : public ui::UiEventObserver,
                                          public state::StateManagerInterface {

  sdl2w::Window* window;
  bmin::String itemName;
  bmin::String itemId;

public:
  ObserverShowLayerInventoryContext(sdl2w::Window* _window,
                                    const bmin::String& itemName,
                                    const bmin::String& itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};
} // namespace ui

} // export

export {

namespace ui {

struct ListMagicSpellsPropsSpell {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  // GCC BMI: DynArray nested in DynArray under UiElement corrupts GCM.
  std::vector<bmin::String> requiredRuneSprites;
};

struct ListMagicSpellsProps {
  bmin::DynArray<ListMagicSpellsPropsSpell> spells;
  bmin::String casterId;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
  bool enableSpellInfoOnClick = true;
  bool enableSpellCastOnClick = false;
};

class ListMagicSpells : public UiElement {
private:
  ListMagicSpellsProps props;
  static constexpr int iconSpriteSize = 24;
  static constexpr float iconScale = 1.f;
  static constexpr int labelGapAfterIcon = 12;
  static constexpr int requiredRuneIconSize = 24;
  static constexpr float requiredRuneIconScale = 1.f;
  static constexpr int requiredRuneGap = 2;
  static constexpr int requiredRunesRightPadding = 4;
  UiElement* createSpellElement(const ListMagicSpellsPropsSpell& spell);

public:
  ListMagicSpells(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListMagicSpells() override = default;
  void setProps(const ListMagicSpellsProps& props);
  const ListMagicSpellsProps& getProps() const;
  const std::pair<int, int> getDims() const override;
  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverSelectSpellCast.hpp ---
namespace ui {

class ObserverSelectSpellCast : public ui::UiEventObserver,
                                public state::StateManagerInterface {
  bmin::String spellId;
  bmin::String chId;

public:
  explicit ObserverSelectSpellCast(const bmin::String& _spellId,
                                   const bmin::String& _chId)
      : spellId(_spellId), chId(_chId) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerSpellInfo.hpp ---
namespace ui {

class ObserverShowLayerSpellInfo : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String spellName;

public:
  ObserverShowLayerSpellInfo(sdl2w::Window* _window, const bmin::String& _spellName)
      : window(_window), spellName(_spellName) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/lists/ListPickUp.h ---
namespace ui {

struct ListPickUpPropsItem {
  model::ItemInstance item;
  bmin::String itemLabel;
  int weight = 1;
  bmin::String itemSprite;
};

struct ListPickUpProps {
  bmin::DynArray<ListPickUpPropsItem> items;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

// ListPickUp - renders a vertical list of items that can be picked up
class ListPickUp : public UiElement {
private:
  ListPickUpProps props;

  static constexpr int contextBtnSize = 32;
  static constexpr int iconSpriteSize = 16;
  static constexpr float iconScale = 2.f;
  static constexpr int maxShortcutItems = 26;
  static constexpr int shortcutGapAfterIcon = 4;
  static constexpr int labelGapAfterShortcut = 8;
  static constexpr int labelGapAfterIconNoShortcut = 24;

  UiElement* createItemElement(const ListPickUpPropsItem& item, int index);
  static bmin::String shortcutLetterForIndex(int index);

public:
  ListPickUp(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListPickUp() override = default;

  void setProps(const ListPickUpProps& props);
  const ListPickUpProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverPickUpItem.hpp ---
namespace ui {

class ObserverPickUpItem : public ui::UiEventObserver,
                           public state::StateManagerInterface {
  bmin::String itemId;

public:
  explicit ObserverPickUpItem(const model::ItemInstance& item) : itemId(item.id) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerPickUpContext.hpp ---
namespace ui {
class ObserverShowLayerPickUpContext : public ui::UiEventObserver,
                                       public state::StateManagerInterface {

  sdl2w::Window* window;
  model::ItemInstance item;

public:
  ObserverShowLayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item)
      : window(_window), item(item) {}

  void onClick(int mouseX, int mouseY, int button) override;
};
} // namespace ui

} // export

export {

// --- from ui/components/ConfirmModal.h ---
namespace ui {


struct ConfirmModalProps {
  bmin::String title = bmin::String(TRANSLATE("Confirm"));
  bmin::String message;
  bmin::String confirmButtonLabel = bmin::String(TRANSLATE("Yes"));
  bmin::String cancelButtonLabel = bmin::String(TRANSLATE("No"));
};

class ConfirmModal : public UiElement {
  ConfirmModalProps props;

  ButtonGroup* buttonGroup = nullptr;

public:
  ConfirmModal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ConfirmModal() override = default;

  void setProps(const ConfirmModalProps& _props);
  ConfirmModalProps& getProps();
  const ConfirmModalProps& getProps() const;

  ButtonGroup* getButtonGroup();
  const ButtonGroup* getButtonGroup() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/FloatingNotification.h ---
namespace ui {

struct FloatingNotificationProps {
  bmin::String id;
  bmin::String message;
  state::UiFloatingNotificationType type = state::UiFloatingNotificationType::INFO;
};

class FloatingNotification : public UiElement {
private:
  static constexpr int kHorizontalPadding = 16;
  static constexpr int kVerticalPadding = 8;

  FloatingNotificationProps props;

  SDL_Color getTextColor() const;

public:
  FloatingNotification(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotification() override = default;

  void setProps(const FloatingNotificationProps& _props);
  const FloatingNotificationProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/borders/BorderInGameNarrow.h ---
namespace ui {

struct BorderInGameNarrowProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int partyMemberAreaHeight = 72;
  int sideBorderWidth = 16;
};

// BorderInGameNarrow component - renders a narrow in-game border layout
class BorderInGameNarrow : public BorderInGame {
private:
  BorderInGameNarrowProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameNarrow() override = default;

  void setProps(const BorderInGameNarrowProps& _props);
  BorderInGameNarrowProps& getProps();
  const BorderInGameNarrowProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui

} // export

export {

// --- from ui/components/borders/BorderInGameWide.h ---
namespace ui {

struct BorderInGameWideProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int subtitleHeight = 24;
  int partyMemberAreaWidth = 76;
  int leftBorderWidth = 16;
};

// BorderInGameWide component - renders a wide in-game border layout
class BorderInGameWide : public BorderInGame {
private:
  BorderInGameWideProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameWide(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameWide() override = default;

  void setProps(const BorderInGameWideProps& _props);
  BorderInGameWideProps& getProps();
  const BorderInGameWideProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui

} // export

export {

// --- from ui/components/borders/BorderModalSmall.h ---
namespace ui {

struct BorderModalSmallProps {
  int width = 0;
  int height = 0;
  int headerHeight = 80;
  int iconSize = 64;
  int borderWidth = 2;
};

class BorderModalSmall : public UiElement {
protected:
  BorderModalSmallProps props;

public:
  BorderModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalSmall() override = default;

  void setProps(const BorderModalSmallProps& _props);
  BorderModalSmallProps& getProps();
  const BorderModalSmallProps& getProps() const;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconBorderLocation() const;
  const std::pair<int, int> getIconLocationCenter() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void buildTiledOverlay();
  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/FloatingNotificationSection.h ---
namespace ui {

struct FloatingNotificationSectionProps {
  int bottomMargin = 40;
  int notificationGap = 8;
};

class FloatingNotificationSection : public UiElement {
private:
  FloatingNotificationSectionProps props;
  std::uint64_t syncedNotificationRevision =
      std::numeric_limits<std::uint64_t>::max();

  template <state::ActionEvent Event, typename Fn> void subscribeAction(Fn&& fn) {
    if (!hasStateManager()) {
      return;
    }
    getStateManager()->getActionBus().subscribe(
        this,
        Event,
        [fn = std::forward<Fn>(fn)](state::AbstractAction& action, state::State& state) {
          fn(action, state);
        });
  }

  void syncFromState(const state::State& state);
  void layoutNotifications();

public:
  FloatingNotificationSection(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotificationSection() override;

  void setProps(const FloatingNotificationSectionProps& _props);
  const FloatingNotificationSectionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/borders/BorderModalStandard.h ---
namespace ui {

// struct BorderModalStandardProps {
//   int topLeftSquareSize = 78;
//   int leftBorderWidth = 16;
//   int borderSize = 4;
//   int closeButtonPadding = 6;
//   int subtitleYOffset = 40;
// };

// BorderModalStandard component - renders a specific border layout using OutsetRectangle
// elements Uses Position, Size, Scale from BaseStyle
class BorderModalStandard : public BorderModalSmall {
private:
  // BorderModalStandardProps props;

public:
  static const int BOTTOM_BORDER_HEIGHT = 10;
  BorderModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalStandard() override = default;

  // void setProps(const BorderModalStandardProps& _props);
  // BorderModalStandardProps& getProps();
  // const BorderModalStandardProps& getProps() const;

  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconSectionCenter() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getSubTitleLocation() const;
  const std::pair<int, int> getSubTitleDims() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/TouchMovePad.h ---
namespace ui {

struct TouchMovePadProps {
  int buttonGapH = 10;
  int buttonGapV = 12;
  int padding = 4;
  int borderSize = 4;
  int dragBarHeight = 18;
};

// TouchMovePad - floating directional pad for mouse/touch movement input.
class TouchMovePad : public UiElement {
  struct ButtonPlacement {
    MoveDirection direction;
    const char* id;
  };

  static constexpr int halfButtonW = 22;
  static constexpr int halfButtonH = 22;
  static constexpr int cardButtonW = 44;
  static constexpr int cardButtonH = 22;
  static constexpr int borderButtonW = 18;
  static constexpr int borderButtonH = 8;

  static constexpr ButtonPlacement buttonPlacements[] = {
      {MoveDirection::UpLeft, "move_up_left"},
      {MoveDirection::Up, "move_up"},
      {MoveDirection::UpRight, "move_up_right"},
      {MoveDirection::Left, "move_left"},
      {MoveDirection::Wait, "move_wait"},
      {MoveDirection::Right, "move_right"},
      {MoveDirection::DownLeft, "move_down_left"},
      {MoveDirection::Down, "move_down"},
      {MoveDirection::DownRight, "move_down_right"},
  };

  TouchMovePadProps props;
  bool isDragging = false;
  int dragOffsetX = 0;
  int dragOffsetY = 0;

  int getGridWidth() const;
  int getGridHeight() const;
  int getContentWidth() const;
  int getContentHeight() const;
  int getBodyY() const;
  int getBodyHeight() const;
  int getWideRowWidth() const;
  int getNarrowRowWidth() const;
  std::pair<int, int> getButtonPosition(MoveDirection direction) const;
  bool isInDragBar(int mouseX, int mouseY) const;
  void positionChildren();
  void syncDragHandle();
  void moveTo(int x, int y);

public:
  TouchMovePad(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TouchMovePad() override = default;

  void setProps(const TouchMovePadProps& _props);
  TouchMovePadProps& getProps();
  const TouchMovePadProps& getProps() const;

  void startDrag(int mouseX, int mouseY);
  void endDrag();

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/lists/ListChCompactInfoHorizontal.h ---
// IWYU pragma: keep

namespace ui {

struct ListChCompactInfoHorizontalProps {
  bmin::DynArray<ChCompactInfoProps> entries;
  int selectedIndex = 0;
  int lineGap = 0;
};

// ListChCompactInfoHorizontal - horizontal list of ChCompactInfo rows.
class ListChCompactInfoHorizontal : public UiElement {
private:
  ListChCompactInfoHorizontalProps props;

public:
  ListChCompactInfoHorizontal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListChCompactInfoHorizontal() override = default;

  void setProps(const ListChCompactInfoHorizontalProps& _props);
  const ListChCompactInfoHorizontalProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/lists/ListChCompactInfoVertical.h ---
// IWYU pragma: keep

namespace ui {

struct ListChCompactInfoVerticalProps {
  bmin::DynArray<ChCompactInfoProps> entries;
  int selectedIndex = 0;
  int lineGap = 0;
};

// ListChCompactInfoVertical - vertical list of ChCompactInfo rows.
class ListChCompactInfoVertical : public UiElement {
private:
  ListChCompactInfoVerticalProps props;

public:
  ListChCompactInfoVertical(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListChCompactInfoVertical() override = default;

  void setProps(const ListChCompactInfoVerticalProps& _props);
  const ListChCompactInfoVerticalProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export
