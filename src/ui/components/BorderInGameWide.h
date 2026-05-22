#pragma once

#include "../UiElement.h"

namespace ui {

struct BorderInGameWideProps {
  int titleHeight = 44;
  int subtitleHeight = 24;
  int partyMemberAreaWidth = 84;
  int leftBorderWidth = 16;
  int outsetBorderSize = 4;
  float actionButtonsScale = 1.f;
};

constexpr int ACTION_BUTTON_SIZE = 32;

// BorderInGameWide component - renders a wide in-game border layout using OutsetRectangle
// elements. Uses Position, Size, Scale from BaseStyle
class BorderInGameWide : public UiElement {
private:
  BorderInGameWideProps props;

public:
  BorderInGameWide(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameWide() override = default;

  void setProps(const BorderInGameWideProps& _props);
  BorderInGameWideProps& getProps();
  const BorderInGameWideProps& getProps() const;

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
