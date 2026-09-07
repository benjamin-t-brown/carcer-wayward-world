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

export module carcer.ui.widgets.views;
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

// --- from ui/components/MapView.h ---
namespace ui {

struct MapViewProps {
  int width = 0;
  int height = 0;
};

// Draws State.world.activeMap tiles (from stitched MapInstances), items,
// characters, and damage particles into a clipped content rect using
// State.world.camera.camX / camY (map pixel space). Does not own or mutate camera.
class MapView : public UiElement, public state::DatabaseInterface {
private:
  MapViewProps props;

  SDL_Color mapFogColor{0, 0, 0, 128};
  SDL_Color mapUnexploredColor{0, 0, 0, 255};
  SDL_Color actionAimFillColor{66, 202, 253, 64};
  SDL_Color actionAimOutlineColor{66, 202, 253, 220};

  bmin::Map<bmin::String, bmin::UniquePtr<sdl2w::Animation>> animations;

  void renderDamageParticles(const model::World& world,
                             sdl2w::Draw& draw,
                             sdl2w::Store& store,
                             int contentX,
                             int contentY,
                             int spriteW,
                             int spriteH, 
                             int fontScale);

  void renderProjectiles(const model::World& world,
                         sdl2w::Draw& draw,
                         sdl2w::Store& store,
                         int contentX,
                         int contentY,
                         int spriteW,
                         int spriteH);

  void addAnimation(const bmin::String& animationName);
  sdl2w::Animation* getAnimation(const bmin::String& animationName);
  sdl2w::Animation* upsertAnimation(const bmin::String& animationName);

public:
  MapView(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MapView() override = default;

  void setProps(const MapViewProps& _props);
  MapViewProps& getProps();
  const MapViewProps& getProps() const;

  // Screen pixel → map tile using the inverse of MapView render math.
  // nullopt if outside content rect or outside map bounds.
  std::optional<model::TileXY> screenToTile(int screenX, int screenY) const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/InGameTitleBar.h ---
namespace ui {

struct InGameTitleBarProps {
  int width = 0;
  int height = 0;
  bmin::String title = "Title";
  int day = 0;
  int food = 0;
  int ap = 0;
  bool showAp = false;

  int buttonSize = 32;
  int buttonSpacing = 0;
  int sectionSpacing = 12;
  int statSpacing = 16;
};

// InGameTitleBar component - title bar for the in-game layout with menu buttons,
// customizable title text, and day/food/ap stats.
class InGameTitleBar : public UiElement {
private:
  InGameTitleBarProps props;

  int getContentHeight() const;
  int getBarCenterY() const;
  int centerTopY(int elementHeight) const;

public:
  InGameTitleBar(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameTitleBar() override = default;

  void setProps(const InGameTitleBarProps& _props);
  InGameTitleBarProps& getProps();
  const InGameTitleBarProps& getProps() const;

  // void addMenuButtonObserver(UiEventObserver* observer);
  // void addHelpButtonObserver(UiEventObserver* observer);

  UiElement* createStatLineRightAligned(const bmin::String& text, int x, int y);

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/ItemInfo.h ---
namespace ui {

struct ItemInfoProps {
  int width = 0;
  bmin::String description;
  int weight = 0;
  int value = 0;
};

// ItemInfo - description, weight, and value for an item template.
class ItemInfo : public UiElement {
private:
  ItemInfoProps props;

  static constexpr int kVertSpacerHeight = 12;

public:
  ItemInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ItemInfo() override = default;

  void setProps(const ItemInfoProps& _props);
  const ItemInfoProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/PartyMemberIconSelector.h ---
// IWYU pragma: keep

namespace ui {

enum class PartyMemberIconSelectorTarget { INVENTORY, PICKUP, MAGIC };

struct PartyMemberIconSelectorProps {
  bmin::DynArray<bmin::String> members;
  int selectedIndex = 0;
  PartyMemberIconSelectorTarget target = PartyMemberIconSelectorTarget::INVENTORY;
  int iconSize = 32;
  int iconGap = 4;

  SDL_Color spriteBgColor = Colors::LightGrey;
  SDL_Color spriteBgColorTopRight = Colors::White;
  SDL_Color spriteBgColorBottomLeft = Colors::ButtonModalGrey2;

  SDL_Color spriteSelectedBgColor = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorBottomLeft = Colors::ButtonModalSelected;
};

class PartyMemberIconSelector : public UiElement {
private:
  PartyMemberIconSelectorProps props;

public:
  PartyMemberIconSelector(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberIconSelector() override = default;

  void setProps(const PartyMemberIconSelectorProps& _props);
  PartyMemberIconSelectorProps& getProps();
  const PartyMemberIconSelectorProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMember.hpp ---
namespace ui {

class ObserverSetCurrentPartyMember : public ui::UiEventObserver,
                                      public state::StateManagerInterface {
  int partyMemberIndex;

public:
  explicit ObserverSetCurrentPartyMember(int _partyMemberIndex)
      : partyMemberIndex(_partyMemberIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMemberInventory.hpp ---
namespace ui {

class ObserverSetCurrentPartyMemberInventory : public ui::UiEventObserver,
                                               public state::StateManagerInterface {
  int partyMemberInventoryIndex;

public:
  explicit ObserverSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMemberMagic.hpp ---
namespace ui {

class ObserverSetCurrentPartyMemberMagic : public ui::UiEventObserver,
                                           public state::StateManagerInterface {
  int partyMemberMagicIndex;

public:
  explicit ObserverSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/PartyMemberSwitcher.h ---
// IWYU pragma: keep

namespace ui {

struct PartyMemberSwitcherProps {
  bmin::String spriteName;
  int partyMemberIndex = 0;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int buttonSize = 36;
  int buttonSpacing = 2;
};

// PartyMemberSwitcher - prev/next buttons flanking a party member sprite.
class PartyMemberSwitcher : public UiElement {
private:
  PartyMemberSwitcherProps props;

public:
  PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberSwitcher() override = default;

  void setProps(const PartyMemberSwitcherProps& _props);
  PartyMemberSwitcherProps& getProps();
  const PartyMemberSwitcherProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverUpdateCurrentPartyMember.hpp ---
namespace ui {

class ObserverUpdateCurrentPartyMember : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  int directionDelta;

public:
  ObserverUpdateCurrentPartyMember(int /*_partyMemberIndex*/, int _directionDelta)
      : directionDelta(_directionDelta) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

