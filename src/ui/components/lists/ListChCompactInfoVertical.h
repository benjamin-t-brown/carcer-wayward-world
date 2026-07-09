#pragma once

#include "ui/UiElement.h"
#include "ui/components/ChCompactInfo.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep

namespace ui {

struct ListChCompactInfoVerticalProps {
  DynArray<ChCompactInfoProps> entries;
  int selectedIndex = 0;
  int lineGap = 0;
};

// ListChCompactInfoVertical - vertical list of ChCompactInfo rows.
class ListChCompactInfoVertical : public UiElement {
private:
  ListChCompactInfoVerticalProps props;

public:
  ListChCompactInfoVertical(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListChCompactInfoVertical() override = default;

  void setProps(const ListChCompactInfoVerticalProps& _props);
  const ListChCompactInfoVerticalProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
