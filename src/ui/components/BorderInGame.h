#pragma once

#include "../UiElement.h"

namespace ui {

// BorderInGame component - renders an in-game border layout using OutsetRectangle
// elements. Uses Position, Size, Scale from BaseStyle
class BorderInGame : public UiElement {
public:
  static const int TITLE_HEIGHT = 44;
  static const int SUBTITLE_HEIGHT = 24;
  static const int PARTY_MEMBER_AREA_WIDTH = 84;
  static const int LEFT_BORDER_WIDTH = 16;
  static const int ACTION_BUTTONS_AREA_HEIGHT = 72;

  static const int OUTSET_BORDER_SIZE = 4;

  BorderInGame(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGame() override = default;

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
