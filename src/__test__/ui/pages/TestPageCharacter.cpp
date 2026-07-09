#include "../../setupTestUi.h"
#include "layers/LayerManager.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "model/instances/CharacterPlayer.h"
#include "state/LayerManagerInterface.h"
#include "ui/UiElement.h"
#include "ui/pages/PageCharacter.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>
#include "bmin/UniquePtr.h"

class TestLayer : public layers::Layer {
  model::CharacterPlayer characterPlayer;

public:
  TestLayer(sdl2w::Window* _window, model::CharacterPlayer _characterPlayer)
      : layers::Layer(_window), characterPlayer(std::move(_characterPlayer)) {
    auto [windowWidth, windowHeight] = window->getDims();

    auto pageCharacter = bmin::makeUnique<ui::PageCharacter>(window);
    pageCharacter->setId("pageCharacter");
    pageCharacter->setPos(0, 0);
    pageCharacter->setProps(ui::PageCharacterProps{
        .width = windowWidth,
        .height = windowHeight,
        .characterPlayer = &characterPlayer,
    });

    addUiElement(pageCharacter.release());
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start PageCharacter test" << LOG_ENDL;
  srand(time(NULL));

  bmin::UniquePtr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PageCharacter test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    auto characterPlayer =
        model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1"));
    layerManager->addLayer(new TestLayer(&window, std::move(characterPlayer)));

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
      argc, argv, TestUiParams{640, 480, "PageCharacter Test"}, _init, _updateRender, [&]() { layerManager.reset(); });
  LOG(INFO) << "End PageCharacter test" << LOG_ENDL;
  return 0;
}
