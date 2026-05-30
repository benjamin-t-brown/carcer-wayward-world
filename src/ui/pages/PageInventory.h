#pragma once

#include "model/instances/CharacterPlayer.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListInventory.h"
#include <string>

namespace ui {

struct PageInventoryPartyMember {
  std::string spriteName;
};

struct PageInventoryProps {
  std::string characterPlayerId;
  std::string characterPlayerLabel;
  std::string characterPlayerSprite;
  int partyMemberInventoryIndex = 0;
  std::vector<PageInventoryPartyMember> partyMembers;
  int weightCarrying = 0;
  int weightCapacity = 0;
  int gold = 0;
  std::vector<model::CharacterInventoryItem> inventory;
  model::CharacterPlayerEquipment equipment;
};

class PageInventory : public UiElement, public state::DatabaseInterface {
private:
  PageInventoryProps props;

  void populateInventoryProps(std::vector<ListInventoryPropsItem>& listProps);

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
