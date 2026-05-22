#pragma once

#include "ui/UiElement.h"
#include "ui/components/ChCompactInfo.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep

namespace ui {

struct ListChCompactInfoHorizontalProps {
  std::vector<ChCompactInfoProps> entries;
  int lineGap = 0;
};

// ListChCompactInfoHorizontal - horizontal list of ChCompactInfo rows.
class ListChCompactInfoHorizontal : public UiElement {
private:
  ListChCompactInfoHorizontalProps props;

public:
  ListChCompactInfoHorizontal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListChCompactInfoHorizontal() override = default;

  void setProps(const ListChCompactInfoHorizontalProps& _props);
  const ListChCompactInfoHorizontalProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
