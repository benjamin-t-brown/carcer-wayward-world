#pragma once

#include "BorderInGame.h"

namespace ui {

struct BorderInGameNarrowProps : BorderInGameProps {
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
