#pragma once

#include "db/Database.h"
#include "model/Items.h"
#include "ui/UiElement.h"
#include <string>
#include <vector>

namespace ui {

struct ListPickUpProps {
  std::vector<std::string> itemNames;
};

// ListPickUp - renders a vertical list of items that can be picked up
class ListPickUp : public UiElement {
private:
  ListPickUpProps props;

public:
  ListPickUp(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListPickUp() override = default;

  void setProps(const ListPickUpProps& props);
  const ListPickUpProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

struct ListPickUpItemProps {
  const model::ItemTemplate* itemTemplate = nullptr;
  std::string itemName;
};
// ListPickUpItem - renders a single pickup item with icon, label, and weight
class ListPickUpItem : public UiElement {
private:
  ListPickUpItemProps props;
  const SDL_Color bgColor = Colors::White;

public:
  ListPickUpItem(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListPickUpItem() override = default;

  // Set the item template data
  void setProps(const ListPickUpItemProps& props);
  const ListPickUpItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

