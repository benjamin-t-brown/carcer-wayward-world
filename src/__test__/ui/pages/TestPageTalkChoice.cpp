#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/pages/PageTalkChoice.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start PageTalkChoice test" << LOG_ENDL;
  srand(time(NULL));

  // Setup static classes
  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();
  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PageTalkChoice test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    auto pageTalkChoice = new ui::PageTalkChoice(&window, nullptr);
    pageTalkChoice->setId("pageTalkChoice");
    auto& style = pageTalkChoice->getStyle();
    auto scale = 1.f;
    style.width = windowWidth / scale;
    style.height = windowHeight / scale;
    style.x = 0;
    style.y = 0;
    style.scale = scale;
    pageTalkChoice->setStyle(style);

    ui::PageTalkChoiceProps pageProps;
    pageProps.title = "Dockmaster Claire";
    pageProps.portraitSpriteName = "";
    pageProps.choices = {
        {.nextId = "choice1", .text = "Choice 1", .prefixText = ""},
        {.nextId = "choice2", .text = "Choice 2", .prefixText = ""},
        {.nextId = "choice3", .text = "Choice 3", .prefixText = "[Special]"},
    };
    // clang-format off
    pageProps.textBlocks = {
      {.text ="According to all known laws of aviation, there is no way a bee should be able to fly.\n"},
      {.text ="Its wings are too small to get its fat little body off the ground.\n"},
      {.text ="The bee, of course, flies anyway because bees don't care what humans think is impossible.\n"},
      {.text ="Yellow, black. Yellow, black. Yellow, black. Yellow, black.\n"},
      {.text ="Ooh, black and yellow!\n"},
      {.text ="Let's shake it up a little.\n"},
      {.text ="Barry! Breakfast is ready!\n"},
    };
    // clang-format on
    pageTalkChoice->setProps(pageProps);

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(pageTalkChoice));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          for (auto& elem : elements) {
            elem->checkMouseDownEvent(x, y, button);
          }
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          for (auto& elem : elements) {
            elem->checkMouseUpEvent(x, y, button);
          }
        });

    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
        [&](int x, int y, int delta) {
          for (auto& elem : elements) {
            elem->checkMouseWheelEvent(x, y, delta);
          }
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    stateManager.update(window.getDeltaTime());
    for (auto& elem : elements) {
      elem->checkHoverEvent(window.getEvents().mouseX, window.getEvents().mouseY);
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();

    // Render all elements
    for (auto& elem : elements) {
      elem->render(window.getDeltaTime());
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{640, 480, "PageTalkChoice Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End PageTalkChoice test" << LOG_ENDL;
  return 0;
}
