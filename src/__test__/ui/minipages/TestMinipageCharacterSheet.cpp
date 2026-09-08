#include "../../setupTestUi.hpp"
#include "layers/LayerManager.h"
#include "layers/UiLayer.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/minipages/MinipageCharacterSheet.h"
#include "ui/SdlPixels.hpp" // IWYU pragma: keep
#include <memory>
#include "bmin/UniquePtr.h"

class TestLayer : public layers::UiLayer {
public:
  TestLayer(sdl2w::Window* _window) : layers::UiLayer(_window) {
    auto [windowWidth, windowHeight] = window->getDims();

    auto minipageCharacterSheet = bmin::makeUnique<ui::MinipageCharacterSheet>(window);
    minipageCharacterSheet->setId("minipageCharacterSheet");
    minipageCharacterSheet->setPos(0, 0);
    minipageCharacterSheet->setProps(ui::MinipageCharacterSheetProps{
        .width = windowWidth,
        .height = windowHeight,
    });

    addUiElement(bmin::UniquePtr<ui::UiElement>(minipageCharacterSheet.release()));
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start MinipageCharacterSheet test" << LOG_ENDL;
  srand(time(NULL));

  bmin::UniquePtr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "MinipageCharacterSheet test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);

    layerManager->addLayer(bmin::UniquePtr<layers::Layer>(new TestLayer(&window)));

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          layerManager->handleMouseDown(x, y, button);
        });
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
          layerManager->handleMouseUp(x, y, button);
        });

    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL, [&](int x, int y, int delta) {
          layerManager->handleMouseWheel(x, y, delta);
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    layerManager->update(window.getDeltaTime());
    stateManager.update(window.getDeltaTime());
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();

    layerManager->render(window.getDeltaTime());
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "MinipageCharacterSheet Test"},
              _init,
              _updateRender, [&]() { layerManager.reset(); });
  LOG(INFO) << "End MinipageCharacterSheet test" << LOG_ENDL;
  return 0;
}
