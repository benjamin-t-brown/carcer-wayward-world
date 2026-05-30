#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/TextBanner.h"
#include "ui/elements/buttons/ButtonModal.h"
#include <memory>

namespace {

ui::TextBannerCorner nextCorner(ui::TextBannerCorner corner) {
  switch (corner) {
  case ui::TextBannerCorner::LEFT_TOP:
    return ui::TextBannerCorner::RIGHT_TOP;
  case ui::TextBannerCorner::RIGHT_TOP:
    return ui::TextBannerCorner::RIGHT_BOTTOM;
  case ui::TextBannerCorner::RIGHT_BOTTOM:
    return ui::TextBannerCorner::LEFT_BOTTOM;
  case ui::TextBannerCorner::LEFT_BOTTOM:
    return ui::TextBannerCorner::LEFT_TOP;
  }
  return ui::TextBannerCorner::LEFT_TOP;
}

class SwitchCornerObserver : public ui::UiEventObserver {
  ui::TextBanner* banner;
  ui::TextBannerCorner* corner;

public:
  SwitchCornerObserver(ui::TextBanner* _banner, ui::TextBannerCorner* _corner)
      : banner(_banner), corner(_corner) {}
  ~SwitchCornerObserver() override = default;

  void onClick(int x, int y, int button) override {
    *corner = nextCorner(*corner);
    auto newProps = banner->getProps();
    newProps.corner = *corner;
    banner->setProps(newProps);
    LOG(INFO) << "Switched TextBanner corner" << LOG_ENDL;
  }
};

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start TextBanner test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;
  const int containerX = 40;
  const int containerY = 40;
  const int containerWidth = 320;
  const int containerHeight = 180;
  ui::TextBannerCorner corner = ui::TextBannerCorner::LEFT_TOP;
  const std::string text = "Hello, world!";

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "TextBanner test initialized" << LOG_ENDL;

    auto banner = new ui::TextBanner(&window);
    banner->setId("banner");
    banner->setProps(ui::TextBannerProps{
        .location = {containerX, containerY},
        .dims = {containerWidth, containerHeight},
        .corner = corner,
        .text = text,
    });
    elements.push_back(std::unique_ptr<ui::UiElement>(banner));

    auto switchCornerButton = new ui::ButtonModal(&window);
    switchCornerButton->setId("switchCorner");
    auto& buttonStyle = switchCornerButton->getStyle();
    buttonStyle.x = containerX;
    buttonStyle.y = containerY + containerHeight + 24;
    buttonStyle.width = 220;
    buttonStyle.height = 40;
    switchCornerButton->setProps(ui::ButtonModalProps{.text = "Next Corner"});
    switchCornerButton->addEventObserver(
        new SwitchCornerObserver(banner, &corner));
    elements.push_back(std::unique_ptr<ui::UiElement>(switchCornerButton));

    auto& events = window.getEvents();
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                         [&](int x, int y, int button) {
                           for (auto& elem : elements) {
                             elem->checkMouseDownEvent(x, y, button);
                           }
                         });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      for (auto& elem : elements) {
        elem->checkMouseUpEvent(x, y, button);
      }
    });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.clearScreen();

    draw.drawRect(
        containerX, containerY, containerWidth, containerHeight, ui::Colors::Black);
    for (auto& element : elements) {
      element->render(window.getDeltaTime());
    }

    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 500, "TextBanner Test"}, _init, _updateRender, [&]() {
        elements.clear();
      });
  LOG(INFO) << "End TextBanner test" << LOG_ENDL;
  return 0;
}
