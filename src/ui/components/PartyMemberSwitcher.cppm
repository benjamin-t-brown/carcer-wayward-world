module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <algorithm>

export module carcer.ui.components:PartyMemberSwitcher;
import carcer.actions.ui.UiSetCurrentPartyMember;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.elements;
#include "macros.h"

export {

// --- from ui/components/PartyMemberSwitcher.h ---
// IWYU pragma: keep

namespace ui {

struct PartyMemberSwitcherProps {
  bmin::String spriteName;
  int partyMemberIndex = 0;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int buttonSize = 36;
  int buttonSpacing = 2;
};

// PartyMemberSwitcher - prev/next buttons flanking a party member sprite.
class PartyMemberSwitcher : public UiElement {
private:
  PartyMemberSwitcherProps props;

public:
  PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberSwitcher() override = default;

  void setProps(const PartyMemberSwitcherProps& _props);
  PartyMemberSwitcherProps& getProps();
  const PartyMemberSwitcherProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverUpdateCurrentPartyMember.hpp ---
namespace ui {

class ObserverUpdateCurrentPartyMember : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  int directionDelta;

public:
  ObserverUpdateCurrentPartyMember(int /*_partyMemberIndex*/, int _directionDelta)
      : directionDelta(_directionDelta) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

namespace ui {

PartyMemberSwitcher::PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PartyMemberSwitcher::setProps(const PartyMemberSwitcherProps& _props) {
  props = _props;
  build();
}

PartyMemberSwitcherProps& PartyMemberSwitcher::getProps() { return props; }

const PartyMemberSwitcherProps& PartyMemberSwitcher::getProps() const { return props; }

const std::pair<int, int> PartyMemberSwitcher::getDims() const {
  const int w = props.buttonSize + props.buttonSpacing + props.spriteBoxSize +
                props.buttonSpacing + props.buttonSize;
  const int h = std::max(props.buttonSize, props.spriteBoxSize);
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void PartyMemberSwitcher::build() {
  children.clear();

  const int buttonSize = props.buttonSize;
  const int spriteSize = props.spriteBoxSize;
  const int spacing = props.buttonSpacing;
  const int totalWidth = buttonSize + spacing + spriteSize + spacing + buttonSize;
  const int totalHeight = std::max(buttonSize, spriteSize);

  style.width = totalWidth;
  style.height = totalHeight;

  auto container = new Quad(window, this);
  container->setPos(style.x, style.y);
  container->setScale(style.scale);
  container->setProps(QuadProps{
      .width = totalWidth,
      .height = totalHeight,
  });
  addChild(container);

  auto leftBtn = new ButtonModal(window, container);
  leftBtn->setId("prevPartyMember");
  leftBtn->setPos(0, (totalHeight - buttonSize) / 2);
  leftBtn->setProps(ButtonModalProps{.text = "<", .width = buttonSize, .height = buttonSize});
  leftBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, -1));
  container->addChild(leftBtn);

  const int spriteX = buttonSize + spacing;

  auto spriteRect = new OutsetRectangle(window, container);
  spriteRect->setPos(spriteX, 0);
  spriteRect->setProps(OutsetRectangleProps{
      .width = spriteSize,
      .height = spriteSize,
      .color = props.spriteBgColor,
      .colorTopRight = props.spriteBorderColor1,
      .colorBottomLeft = props.spriteBorderColor2,
      .borderSize = props.spriteBorderSize,
  });
  container->addChild(spriteRect);

  auto sprite = new Quad(window, container);
  sprite->setPos(spriteX + 1, 1);
  sprite->setProps(QuadProps{
      .width = spriteSize,
      .height = spriteSize,
      .bgSprite = props.spriteName,
  });
  container->addChild(sprite);

  auto rightBtn = new ButtonModal(window, container);
  rightBtn->setId("nextPartyMember");
  rightBtn->setPos(spriteX + spriteSize + spacing, (totalHeight - buttonSize) / 2);
  rightBtn->setProps(ButtonModalProps{.text = ">", .width = buttonSize, .height = buttonSize});
  rightBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, 1));
  container->addChild(rightBtn);
}

void PartyMemberSwitcher::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverUpdateCurrentPartyMember::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverUpdateCurrentPartyMember::onClick delta=" << directionDelta
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }

    auto& player = stateManager->getState().player;
    const int partySize = static_cast<int>(player.party.size());
    if (partySize == 0) {
      return;
    }

    int nextIndex = player.currentPartyMemberIndex + directionDelta;
    if (nextIndex < 0) {
      nextIndex = partySize - 1;
    } else if (nextIndex >= partySize) {
      nextIndex = 0;
    }

    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::UiSetCurrentPartyMember(nextIndex),
                                0);
  }

} // namespace ui
