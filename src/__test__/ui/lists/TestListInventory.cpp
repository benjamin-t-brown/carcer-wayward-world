#include "../../setupTestUi.h"
#include "layers/LayerManager.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "state/LayerManagerInterface.h"
#include "ui/UiElement.h"
#include "ui/lists/ListInventory.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

class TestLayer : public layers::Layer {
public:
  TestLayer(sdl2w::Window* _window) : layers::Layer(_window) {
    model::CharacterPlayer characterPlayer;

    std::vector<std::string>
        //
        itemNames = {"PotionHealing",
                     "DaggerBronze",
                     "ShortSwordBronze",
                     "SwordBronze",
                     "LongbowOak",
                     "ArrowsStone",
                     "ShirtSimple0",
                     "ShirtSimple1",
                     "PantsSimple0",
                     "GlovesLeather",
                     "HatLeather",
                     "BootsLeather",
                     "NecklaceSilver"};

    for (const auto& itemName : itemNames) {
      characterPlayer.addItemToInventory(getDatabase()->getItemTemplate(itemName), 1);
    }

    getStateManager()->getState().player.party.push_back(characterPlayer);

    // auto [windowWidth, windowHeight] = window.getDims();

    // Create ListInventory component
    auto listInventory = std::make_unique<ui::ListInventory>(window);
    listInventory->setId("listInventory");
    ui::BaseStyle style = listInventory->getStyle();
    style.width = 500;
    style.x = 100;
    style.y = 50;
    listInventory->setStyle(style);

    ui::ListInventoryProps listInventoryProps;
    listInventoryProps.character = &getStateManager()->getState().player.party[0];
    listInventory->setProps(listInventoryProps);

    addUiElement(listInventory.release());
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start ListInventory test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  // layers::LayerManager layerManager(&window);
  std::unique_ptr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  // std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ListInventory test initialized" << LOG_ENDL;

    layerManager = std::make_unique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    layerManager->addLayer(new TestLayer(&window));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          //
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          layerManager->handleMouseDown(x, y, button);
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          //
          layerManager->handleMouseUp(x, y, button);
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

    // Render all elements
    layerManager->render(window.getDeltaTime());
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "ListInventory Test"}, _init, _updateRender);
  LOG(INFO) << "End ListInventory test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}
