#include "../../../setupTestUi.h"
#include "lib/sdl2w/Defines.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListVerticalChCompactInfo.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start ListVerticalChCompactInfo test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ListVerticalChCompactInfo test initialized" << LOG_ENDL;

    auto list = new ui::ListVerticalChCompactInfo(&window);
    auto& style = list->getStyle();
    style.width = 260;
    style.x = 120;
    style.y = 80;
    style.scale = 1.0f;

    ui::ListVerticalChCompactInfoProps listProps;

    {
      ui::ChCompactInfoProps entry;
      entry.characterSpriteName = "actors0_0";
      entry.statusEffectSpriteNames = {
          "ui_status_effect_icons_0",
          "ui_status_effect_icons_1",
      };
      entry.hp = 84;
      entry.mana = 37;
      listProps.entries.push_back(entry);
    }
    {
      ui::ChCompactInfoProps entry;
      entry.characterSpriteName = "actors0_4";
      entry.statusEffectSpriteNames = {
          "ui_status_effect_icons_2",
          "ui_status_effect_icons_3",
          "ui_status_effect_icons_4",
          "ui_status_effect_icons_1",
      };
      entry.hp = 12;
      entry.mana = 99;
      listProps.entries.push_back(entry);
    }
    {
      ui::ChCompactInfoProps entry;
      entry.characterSpriteName = "actors0_8";
      entry.hp = 200;
      entry.mana = 5;
      listProps.entries.push_back(entry);
    }

    list->setProps(listProps);
    elements.push_back(std::unique_ptr<ui::UiElement>(list));

    auto list2 = new ui::ListVerticalChCompactInfo(&window);
    list2->setStyle(list->getStyle());
    list2->getStyle().x = 370;
    list2->getStyle().scale = 2.f;
    list2->getStyle().fontSize = sdl2w::TEXT_SIZE_36;
    list2->setProps(list->getProps());
    elements.push_back(std::unique_ptr<ui::UiElement>(list2));
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
              TestUiParams{800, 600, "ListVerticalChCompactInfo Test"},
              _init,
              _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ListVerticalChCompactInfo test" << LOG_ENDL;
  return 0;
}
