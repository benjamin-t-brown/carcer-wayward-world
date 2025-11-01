#include "../../setupTestUi.h"
#include "db/spriteMappings.h"
#include "layers/LayerManager.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "state/LayerManagerInterface.h"
#include "ui/UiElement.h"
#include "ui/popups/PopupInventoryItem.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

class TestLayer : public layers::Layer {
public:
  TestLayer(sdl2w::Window* _window) : layers::Layer(_window) {
    getDatabase()->addItemTemplate(model::ItemTemplate{
        .name = "TestItem1",
        .label = "Test Item 1",
        .iconSpriteName = db::getItemIconSpriteName("potionGreen"),
        .description = "This is an example test item description.  It should gracefully "
                       "wrap in the tooltip and look pretty good in the process.",
        .weight = 999,
        .value = 8888,
    });
    // model::CharacterPlayer characterPlayer;
    // characterPlayer.addItemToInventory(getDatabase()->getItemTemplate("TestItem1"), 1);
    // getStateManager()->getState().player.party.push_back(characterPlayer);

    // Create PopupInventoryItem component
    auto popupInventoryItem = std::make_unique<ui::PopupInventoryItem>(window, this);
    popupInventoryItem->setId("popupInventoryItem");
    ui::BaseStyle style = popupInventoryItem->getStyle();
    style.width = 400;
    style.height = 300;
    style.x = 200;
    style.y = 150;
    style.scale = 1.0f;
    popupInventoryItem->setStyle(style);

    ui::PopupInventoryItemProps popupProps;
    popupProps.itemName = "PotionHealing";
    popupProps.itemId = "";
    popupInventoryItem->setProps(popupProps);

    addUiElement(std::move(popupInventoryItem));
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start PopupInventoryItem test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::unique_ptr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PopupInventoryItem test initialized" << LOG_ENDL;

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

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "PopupInventoryItem Test"},
              _init,
              _updateRender);
  LOG(INFO) << "End PopupInventoryItem test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}
