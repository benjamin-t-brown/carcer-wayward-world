#pragma once

#include "model/instances/CharacterPlayer.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListInventory.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace ui {

struct PageInventoryPartyMember {
  bmin::String spriteName;
};

struct PageInventoryProps {
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::String characterPlayerSprite;
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
