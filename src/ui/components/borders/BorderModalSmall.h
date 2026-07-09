#pragma once

#include "../../UiElement.h"

namespace ui {

struct BorderModalSmallProps {
  int width = 0;
  int height = 0;
  int headerHeight = 80;
  int iconSize = 64;
  int borderWidth = 2;
};

class BorderModalSmall : public UiElement {
protected:
  BorderModalSmallProps props;

public:
  BorderModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalSmall() override = default;

  void setProps(const BorderModalSmallProps& _props);
  BorderModalSmallProps& getProps();
  const BorderModalSmallProps& getProps() const;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconBorderLocation() const;
  const std::pair<int, int> getIconLocationCenter() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void build() override;
  void renderBgOverlay();
  void render(int dt) override;
};

} // namespace ui
