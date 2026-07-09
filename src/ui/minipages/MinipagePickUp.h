#pragma once

#include "lib/sdl2w/L10n.h"
#include "state/DatabaseInterface.h"
#include "ui/UiElement.h"
#include "lib/Types.h"

namespace ui {

struct MinipagePickUpProps {
  String titleText = TRANSLATE("Pick Up");
  String statusText;
  String weightText = "Carrying 0/100";
  int partyMemberIndex = 0;
  DynArray<String> partyMemberSprites;
  DynArray<model::ItemInstance> nearbyItems;
  String doneButtonRemoveLayerId;
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
