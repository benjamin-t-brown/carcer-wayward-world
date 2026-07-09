#pragma once

#include "sdl2w/L10n.h"
#include "state/DatabaseInterface.h"
#include "ui/UiElement.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

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
