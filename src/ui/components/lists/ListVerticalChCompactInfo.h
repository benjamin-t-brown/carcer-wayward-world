#pragma once

#include "ui/UiElement.h"
#include "ui/components/ChCompactInfo.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep

namespace ui {

struct ListVerticalChCompactInfoProps {
  std::vector<ChCompactInfoProps> entries;
  int lineGap = 0;
};

// ListVerticalChCompactInfo - vertical list of ChCompactInfo rows.
class ListVerticalChCompactInfo : public UiElement {
private:
  ListVerticalChCompactInfoProps props;

public:
  ListVerticalChCompactInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListVerticalChCompactInfo() override = default;

  void setProps(const ListVerticalChCompactInfoProps& _props);
  const ListVerticalChCompactInfoProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
