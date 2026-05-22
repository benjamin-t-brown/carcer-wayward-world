#pragma once

#include "../UiElement.h"

namespace ui {

struct BorderInGameNarrowProps {
  int titleHeight = 44;
  int partyMemberAreaHeight = 64;
  int sideBorderWidth = 16;
  int outsetBorderSize = 4;
  float actionButtonsScale = 1.f;
};

// BorderInGameNarrow component - renders a narrow in-game border layout using
// OutsetRectangle elements. Uses Position, Size, Scale from BaseStyle
class BorderInGameNarrow : public UiElement {
private:
  BorderInGameNarrowProps props;

public:
  BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameNarrow() override = default;

  void setProps(const BorderInGameNarrowProps& _props);
  BorderInGameNarrowProps& getProps();
  const BorderInGameNarrowProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getTitleDims() const;
  const std::pair<int, int> getPartyMemberAreaLocation() const;
  const std::pair<int, int> getActionButtonsAreaLocation() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
