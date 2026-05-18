#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/GameChStatus.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start GameChStatus test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "GameChStatus test initialized" << LOG_ENDL;

    auto gameChStatus = std::make_unique<ui::GameChStatus>(&window);
    ui::BaseStyle style;
    style.width = 260;
    style.height = 88;
    style.x = 270;
    style.y = 220;
    style.scale = 1.0f;
    gameChStatus->setStyle(style);

    ui::GameChStatusProps props;
    props.characterSpriteName = "ui_item_icons_12";
    props.statusEffectSpriteNames = {
        "ui_item_icons_0",
        "ui_item_icons_8",
        "ui_item_icons_14",
        "ui_item_icons_17",
        "ui_item_icons_29",
        "ui_item_icons_36",
    };
    props.health = 84;
    props.mana = 37;
    props.statusGridCols = 4;
    gameChStatus->setProps(props);

    elements.push_back(std::move(gameChStatus));
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {};

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.clearScreen();

    for (auto& element : elements) {
      element->render(window.getDeltaTime());
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "GameChStatus Test"}, _init, _updateRender);
  LOG(INFO) << "End GameChStatus test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}
