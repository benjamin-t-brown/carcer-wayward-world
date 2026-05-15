#pragma once

#include "../UiElement.h"

namespace ui {

struct BorderInGameProps {
  int titleHeight = 44;
  int subtitleHeight = 24;
  int partyMemberAreaWidth = 84;
  int leftBorderWidth = 16;
  int outsetBorderSize = 4;
  int actionButtonsAreaHeight = 72;
};

// BorderInGame component - renders an in-game border layout using OutsetRectangle
// elements. Uses Position, Size, Scale from BaseStyle
class BorderInGame : public UiElement {
private:
  BorderInGameProps props;

public:
  BorderInGame(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGame() override = default;

  void setProps(const BorderInGameProps& _props);
  BorderInGameProps& getProps();
  const BorderInGameProps& getProps() const;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getSubTitleLocation() const;
  const std::pair<int, int> getPartyMemberAreaLocation() const;
  const std::pair<int, int> getActionButtonsAreaLocation() const;
  const std::pair<int, int> getContentAreaLocation() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
