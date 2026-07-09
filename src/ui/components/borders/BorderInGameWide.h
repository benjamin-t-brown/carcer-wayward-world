#pragma once

#include "BorderInGame.h"

namespace ui {

struct BorderInGameWideProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int subtitleHeight = 24;
  int partyMemberAreaWidth = 76;
  int leftBorderWidth = 16;
};

// BorderInGameWide component - renders a wide in-game border layout
class BorderInGameWide : public BorderInGame {
private:
  BorderInGameWideProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameWide(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameWide() override = default;

  void setProps(const BorderInGameWideProps& _props);
  BorderInGameWideProps& getProps();
  const BorderInGameWideProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui
