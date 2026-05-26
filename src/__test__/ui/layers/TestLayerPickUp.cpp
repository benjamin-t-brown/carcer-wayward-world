#include "../../setupTestUi.h"
#include "db/Database.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerPickUp.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "model/Character.h"
#include "state/DatabaseInterface.h"
#include "state/LayerManagerInterface.h"
#include "state/StateManagerInterface.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr const char* TEST_PARTY_TEMPLATE_NAMES[] = {
    "testPartyMember1",
    "testPartyMember2",
    "testPartyMember3",
    "testPartyMember4",
    "testPartyMember5",
    "testPartyMember6",
};

// const std::vector<std::string> GROUND_PICKUP_ITEMS = {
//     "PotionHealing",
//     "DaggerBronze",
//     "ShortSwordBronze",
//     "SwordBronze",
//     "LongbowOak",
// };

const std::vector<std::vector<std::string>> PARTY_MEMBER_ITEMS = {
    {"PotionHealing", "DaggerBronze"},
    {"ShortSwordBronze", "SwordBronze"},
    {"LongbowOak", "ArrowsStone", "PotionHealing"},
    {"ShirtSimple0", "PantsSimple0"},
    {"GlovesLeather", "HatLeather"},
    {"BootsLeather", "NecklaceSilver", "DaggerBronze"},
};

void setupTestParty(model::Player& player, db::Database& database) {
  player.party.clear();
  player.currentPartyMemberIndex = 0;

  for (size_t i = 0; i < PARTY_MEMBER_ITEMS.size(); ++i) {
    model::CharacterPlayer member;
    member.templateName = TEST_PARTY_TEMPLATE_NAMES[i];

    for (const auto& itemName : PARTY_MEMBER_ITEMS[i]) {
      model::characterPlayerAddItemToInventory(
          member, database.getItemTemplate(itemName), 1);
    }

    player.party.push_back(std::move(member));
  }
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start LayerPickUp integration test" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  setupTestParty(stateManager.getState().player, database);

  std::unique_ptr<layers::LayerManager> layerManager;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "LayerPickUp test initialized" << LOG_ENDL;

    layerManager = std::make_unique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    auto* layerPickUp = new layers::LayerPickUp(&window);
    // layerPickUp->setPickUpItemNames(GROUND_PICKUP_ITEMS);
    layerManager->addLayer(layerPickUp);

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) { layerManager->handleMouseDown(x, y, button); });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      layerManager->handleMouseUp(x, y, button);
    });
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
        [&](int x, int y, int delta) { layerManager->handleMouseWheel(x, y, delta); });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    layerManager->update(window.getDeltaTime());
    stateManager.update(window.getDeltaTime());

    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "LayerPickUp Test"},
              _init,
              _updateRender,
              [&]() { layerManager.reset(); });

  LOG(INFO) << "End LayerPickUp integration test" << LOG_ENDL;
  return 0;
}
