#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include <memory>

class TestButtonGroupObserver : public ui::UiEventObserver {
  std::string id;

public:
  explicit TestButtonGroupObserver(std::string _id) : id(std::move(_id)) {}
  ~TestButtonGroupObserver() override = default;

  void onClick(int x, int y, int button) override {
    LOG(INFO) << "TestButtonGroupObserver click id=" << id << " at: " << x << ", " << y
              << " button: " << button << LOG_ENDL;
  }
};

static void addButtonGroup(sdl2w::Window& window,
                           std::vector<std::unique_ptr<ui::UiElement>>& elements,
                           const std::string& id,
                           int x,
                           int y,
                           ui::ButtonGroupAlignment alignment,
                           const std::vector<std::string>& labels) {

  std::vector<ui::ButtonGroupButtonProps> buttons;
  for (size_t i = 0; i < labels.size(); i++) {
    buttons.push_back(ui::ButtonGroupButtonProps{
        .label = labels[i],
        .type = ui::ButtonGroupButtonType::MODAL,
    });
  }
  auto group = new ui::ButtonGroup(&window);
  group->setId(id);
  auto& groupStyle = group->getStyle();
  groupStyle.x = x;
  groupStyle.y = y;
  groupStyle.scale = 1.f;
  group->setProps(ui::ButtonGroupProps{
      .alignment = alignment,
      .buttonWidth = 100,
      .buttonHeight = 50,
      .buttonSpacing = 8,
      .buttons = buttons,
  });
  for (size_t i = 0; i < labels.size(); i++) {
    group->addObserverToButtonAtIndex(static_cast<int>(i),
                                      new TestButtonGroupObserver(id + ":" + labels[i]));
  }
  elements.push_back(std::unique_ptr<ui::UiElement>(group));
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonGroup test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonGroup test initialized" << LOG_ENDL;

    auto [windowWidth, _] = window.getDims();
    const int rightAnchorX = windowWidth - 16;

    addButtonGroup(window,
                   elements,
                   "leftGroup",
                   50,
                   50,
                   ui::ButtonGroupAlignment::LEFT,
                   {"One", "Two", "Three"});

    addButtonGroup(window,
                   elements,
                   "centerGroup",
                   windowWidth / 2,
                   150,
                   ui::ButtonGroupAlignment::CENTER,
                   {"Center A", "Center B"});

    addButtonGroup(window,
                   elements,
                   "rightSingle",
                   rightAnchorX,
                   250,
                   ui::ButtonGroupAlignment::RIGHT,
                   {"Done"});

    addButtonGroup(window,
                   elements,
                   "rightGroup",
                   rightAnchorX,
                   350,
                   ui::ButtonGroupAlignment::RIGHT,
                   {"Apply", "Cancel"});

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
    auto& events = window.getEvents();
    for (auto& elem : elements) {
      elem->checkHoverEvent(events.mouseX, events.mouseY);
      elem->render(window.getDeltaTime());
    }
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "ButtonGroup Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End ButtonGroup test" << LOG_ENDL;
  return 0;
}
