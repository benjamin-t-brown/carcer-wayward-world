#pragma once

#include "../UiElement.h"
#include "state/WorldActions.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/components/ChCompactInfo.h"
#include <vector>

namespace ui {

class BorderInGame;
class Quad;

enum class InGameBorderType { Wide, Narrow };

// InGameLayout layout properties
struct InGameLayoutProps {
  std::vector<state::WorldActionType> worldActionTypes;
  std::vector<ChCompactInfoProps> partyMembers;
  float actionButtonScale = 1.f;
  InGameBorderType borderType = InGameBorderType::Wide;
};

// InGameLayout layout - renders an in-game layout with background, border, title,
// subtitle, and children Uses Position, Size, Scale from BaseStyle
class InGameLayout : public UiElement {
private:
  InGameLayoutProps props;

  void applyTitleLayout(UiElement* titleElement, BorderInGame* border);
  void buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                          int actionButtonsQuadWidth,
                          int actionButtonsQuadHeight,
                          Quad* actionButtonsQuad);
  void buildChList(const std::pair<int, int>& chListLocation);

public:
  InGameLayout(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameLayout() override = default;

  void setProps(const InGameLayoutProps& _props);
  InGameLayoutProps& getProps();
  const InGameLayoutProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  const std::pair<int, int> getWorldDims();
  const std::pair<int, int> getWorldLocation();
  const std::pair<int, int> getChListLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
