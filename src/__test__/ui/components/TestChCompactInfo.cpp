#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/ChCompactInfo.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start ChCompactInfo test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ChCompactInfo test initialized" << LOG_ENDL;

    {
      auto chCompactInfo = new ui::ChCompactInfo(&window);
      chCompactInfo->setPos(170, 220);
      chCompactInfo->setScale(2.0f);

      ui::ChCompactInfoProps props;
      props.characterSpriteName = "actors0_0";
      props.statusEffectSpriteNames = {
          "ui_status_effect_icons_0",
          "ui_status_effect_icons_1",
          "ui_status_effect_icons_2",
          "ui_status_effect_icons_3",
          "ui_status_effect_icons_4",
          "ui_status_effect_icons_5",
      };
      props.hp = 84;
      props.mana = 37;
      chCompactInfo->setProps(props);

      elements.pushBack(bmin::UniquePtr<ui::UiElement>(chCompactInfo));
    }

    {
      auto chCompactInfo = new ui::ChCompactInfo(&window);
      chCompactInfo->setPos(370, 220);
      chCompactInfo->setScale(1.0f);

      ui::ChCompactInfoProps props;
      props.characterSpriteName = "actors0_6";
      props.statusEffectSpriteNames = {
          "ui_status_effect_icons_2",
          "ui_status_effect_icons_3",
          "ui_status_effect_icons_4",
          "ui_status_effect_icons_2",
          "ui_status_effect_icons_2",
      };
      props.hp = 12;
      props.mana = 99;
      chCompactInfo->setProps(props);

      elements.pushBack(bmin::UniquePtr<ui::UiElement>(chCompactInfo));
    }
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

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "ChCompactInfo Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End ChCompactInfo test" << LOG_ENDL;
  return 0;
}
