#include "../../../setupTestUi.h"
#include "lib/sdl2w/Defines.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListChCompactInfoVertical.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start ListChCompactInfoVertical test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ListChCompactInfoVertical test initialized" << LOG_ENDL;

    auto list = new ui::ListChCompactInfoVertical(&window);
    auto& style = list->getStyle();
    // style.width = 260;
    style.x = 120;
    style.y = 80;
    style.scale = 1.0f;

    ui::ListChCompactInfoVerticalProps listProps;

    {
      ui::ChCompactInfoProps entry;
      entry.characterSpriteName = "actors0_0";
      entry.statusEffectSpriteNames = {
          "ui_status_effect_icons_0",
          "ui_status_effect_icons_1",
      };
      entry.hp = 84;
      entry.mana = 37;
      listProps.entries.pushBack(entry);
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
      listProps.entries.pushBack(entry);
    }
    {
      ui::ChCompactInfoProps entry;
      entry.characterSpriteName = "actors0_8";
      entry.hp = 200;
      entry.mana = 5;
      listProps.entries.pushBack(entry);
    }

    list->setProps(listProps);
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(list));

    auto list2 = new ui::ListChCompactInfoVertical(&window);
    list2->setStyle(list->getStyle());
    list2->getStyle().x = 370;
    list2->getStyle().scale = 2.f;
    list2->getStyle().fontSize = sdl2w::TEXT_SIZE_36;
    list2->setProps(list->getProps());
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(list2));
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
              TestUiParams{800, 600, "ListChCompactInfoVertical Test"},
              _init,
              _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ListChCompactInfoVertical test" << LOG_ENDL;
  return 0;
}
