#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
import carcer.ui.screens;
import carcer.ui.layers;
import sdl2w;
import bmin.string_interop;
#include "macros.h"
#include "../../setupTestUi.h"

class TestLayer : public layers::Layer {
public:
  TestLayer(sdl2w::Window* _window) : layers::Layer(_window) {
    auto [windowWidth, windowHeight] = window->getDims();

    auto minipageEvent = bmin::makeUnique<ui::MinipageEvent>(window);
    minipageEvent->setId("minipageEvent");
    minipageEvent->setPos(0, 0);
    minipageEvent->setProps(ui::MinipageEventProps{
        .width = windowWidth,
        .height = windowHeight,
    });
    addUiElement(minipageEvent.release());
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start MinipageEvent test" << LOG_ENDL;
  srand(time(NULL));

  bmin::UniquePtr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "MinipageEvent test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());
    layerManager->addLayer(new TestLayer(&window));

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
    draw.setBackgroundColor({100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "MinipageEvent Test"}, _init, _updateRender, [&]() { layerManager.reset(); });
  LOG(INFO) << "End MinipageEvent test" << LOG_ENDL;
  return 0;
}
