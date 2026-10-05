#pragma once

#include "model/instances/CharacterPlayer.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListInventory.h"
#include "ui/components/lists/ListSpecialItems.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace ui {

struct PageInventoryPartyMember {
  bmin::String spriteName;
};

struct PageInventorySpecialItem {
  bmin::String itemName;
  bmin::String itemLabel;
  bmin::String itemSprite;
  int quantity = 1;
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
  bmin::DynArray<PageInventorySpecialItem> specialItems;
};

class PageInventory : public UiElement, public state::DatabaseInterface {
private:
  PageInventoryProps props;
  bool showingSpecialItems = false;
  // Defer rebuild after key-button click so observers are not destroyed mid-dispatch.
  bool specialItemsNeedsRebuild = false;

  static constexpr int specialItemsButtonSize = 32;
  // Match PartyMemberIconSelectorProps::iconGap.
  static constexpr int specialItemsButtonGap = 4;
  static constexpr const char* specialItemsButtonSprite = "ui_item_icons_20";

  void populateInventoryProps(bmin::DynArray<ListInventoryPropsItem>& listProps);
  void populateSpecialItemsListProps(bmin::DynArray<ListSpecialItemsPropsItem>& listProps);

public:
  PageInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageInventory() override = default;

  void setProps(const PageInventoryProps& _props);
  PageInventoryProps& getProps();
  const PageInventoryProps& getProps() const;

  void toggleSpecialItemsView();
  // Select special-items view (e.g. keyboard 0). No-op if already showing.
  void selectSpecialItemsView();
  // Leave special-items view (e.g. when a party member is chosen, even same index).
  void exitSpecialItemsView();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
