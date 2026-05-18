#pragma once

#include "ui/UiElement.h"
#include <string>
#include <vector>

namespace ui {

// MinipagePickUp-specific properties
struct MinipagePickUpProps {
  std::vector<std::string> itemNames;
};

// MinipagePickUp - renders the pickup minipage with ModalSmall layout containing
// a scrollable list of items that can be picked up.
class MinipagePickUp : public UiElement {
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
