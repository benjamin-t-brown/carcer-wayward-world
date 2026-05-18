#include "../../setupTestUi.h"
#include "layers/LayerManager.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "state/LayerManagerInterface.h"
#include "ui/UiElement.h"
#include "ui/minipages/MinipagePickUp.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

class TestLayer : public layers::Layer {
public:
  TestLayer(sdl2w::Window* _window) : layers::Layer(_window) {
    std::vector<std::string> itemNames = {"PotionHealing",
                                          "DaggerBronze",
                                          "ShortSwordBronze",
                                          "SwordBronze",
                                          "LongbowOak"};

    auto [windowWidth, windowHeight] = window->getDims();

    auto minipagePickUp = std::make_unique<ui::MinipagePickUp>(window);
    minipagePickUp->setId("minipagePickUp");
    ui::BaseStyle style = minipagePickUp->getStyle();
    style.width = 500;
    style.height = windowHeight - 50;
    style.x = (windowWidth - style.width) / 2;
    style.y = (windowHeight - style.height) / 2;
    minipagePickUp->setStyle(style);

    ui::MinipagePickUpProps minipageProps;
    minipageProps.itemNames = itemNames;
    minipagePickUp->setProps(minipageProps);

    addUiElement(minipagePickUp.release());
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start MinipagePickUp test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::unique_ptr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "MinipagePickUp test initialized" << LOG_ENDL;

    layerManager = std::make_unique<layers::LayerManager>(&window);
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
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "MinipagePickUp Test"}, _init, _updateRender);
  LOG(INFO) << "End MinipagePickUp test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}
