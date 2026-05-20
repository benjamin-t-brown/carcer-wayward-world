#include "../../setupTestUi.h"
#include "db/loaders/LoadSpecialEvents.h"
#include "layers/LayerManager.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "state/LayerManagerInterface.h"
#include "ui/UiElement.h"
#include "ui/pages/PageTalkChoice.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>
#include <unordered_map>

class TestLayer : public layers::Layer {
public:
  TestLayer(sdl2w::Window* _window) : layers::Layer(_window) {
    std::unordered_map<std::string, model::GameEvent> specialEvents;
    std::vector<std::string> eventsToLoad = {"alinea_claire"};
    db::loadSpecialEvents("assets/db/special-events.json", specialEvents, eventsToLoad);

    auto [windowWidth, windowHeight] = window->getDims();

    auto pageTalkChoice = std::make_unique<ui::PageTalkChoice>(window);
    pageTalkChoice->setId("pageTalkChoice");
    ui::BaseStyle style = pageTalkChoice->getStyle();
    style.width = windowWidth;
    style.height = windowHeight;
    style.x = 0;
    style.y = 0;
    pageTalkChoice->setStyle(style);

    ui::PageTalkChoiceProps pageProps;
    pageProps.gameEvent = specialEvents.at("alinea_claire");
    pageProps.portraitName = "Dockmaster Claire";
    pageTalkChoice->setProps(pageProps);

    addUiElement(pageTalkChoice.release());
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start PageTalkChoice test" << LOG_ENDL;
  srand(time(NULL));

  std::unique_ptr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PageTalkChoice test initialized" << LOG_ENDL;

    layerManager = std::make_unique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    layerManager->addLayer(new TestLayer(&window));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          layerManager->handleMouseDown(x, y, button);
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          layerManager->handleMouseUp(x, y, button);
        });

    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
        [&](int x, int y, int delta) {
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
              TestUiParams{640, 480, "PageTalkChoice Test"},
              _init,
              _updateRender, [&]() { layerManager.reset(); });
  LOG(INFO) << "End PageTalkChoice test" << LOG_ENDL;
  return 0;
}
