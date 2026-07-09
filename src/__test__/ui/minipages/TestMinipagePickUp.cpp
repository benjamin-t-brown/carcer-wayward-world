#include "../../setupTestUi.h"
#include "db/Database.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "model/templates/UtilityTypes.h"
#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/minipages/MinipagePickUp.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start MinipagePickUp test" << LOG_ENDL;
  srand(time(NULL));

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  DynArray<UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "MinipagePickUp test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    auto scale = 1.f;
    auto minipagePickUp = new ui::MinipagePickUp(&window);
    minipagePickUp->setId("minipagePickUp");
    auto& style = minipagePickUp->getStyle();
    style.width = 500 / scale;
    style.height = (windowHeight - 50) / scale;
    style.x = (windowWidth - style.width * scale) / 2;
    style.y = (windowHeight - style.height * scale) / 2;
    style.scale = scale;

    minipagePickUp->setProps({
        .statusText = "Too heavy!",
        .weightText = "Carrying 199/500",
        .nearbyItems =
            {
                // clang-format off
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "PotionHealing",
                    .quantity = 5,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "DaggerBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "ShortSwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "SwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "LongbowOak",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "PotionHealing",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "DaggerBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "ShortSwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "SwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "LongbowOak",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "PotionHealing",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "DaggerBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "ShortSwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "SwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "LongbowOak",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "PotionHealing",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "DaggerBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "ShortSwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "SwordBronze",
                    .quantity = 1,
                },
                model::ItemInstance{
                    .id = model::createRandomId(),
                    .itemTemplateName = "LongbowOak",
                    .quantity = 1,
                },
                // clang-format on
            },
    });

    elements.pushBack(UniquePtr<ui::UiElement>(minipagePickUp));

    auto& events = window.getEvents();
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                         [&](int x, int y, int button) {
                           LOG(INFO) << "Mouse down at: " << x << ", " << y
                                     << " - button: " << button << LOG_ENDL;
                           for (auto& elem : elements) {
                             elem->checkMouseDownEvent(x, y, button);
                           }
                         });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      for (auto& elem : elements) {
        elem->checkMouseUpEvent(x, y, button);
      }
    });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
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
              TestUiParams{800, 600, "MinipagePickUp Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End MinipagePickUp test" << LOG_ENDL;
  return 0;
}
