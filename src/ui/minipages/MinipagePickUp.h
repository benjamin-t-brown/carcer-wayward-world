#pragma once

#include "lib/sdl2w/L10n.h"
#include "state/DatabaseInterface.h"
#include "ui/UiElement.h"
#include <string>
#include <vector>

namespace ui {

struct MinipagePickUpProps {
  std::string titleText = TRANSLATE("Pick Up");
  std::string statusText;
  std::string weightText = "Carrying 0/100";
  int partyMemberIndex = 0;
  std::vector<std::string> partyMemberSprites;
  std::vector<model::ItemInstance> nearbyItems;
  std::string doneButtonRemoveLayerId;
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
