#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <cassert>
import carcer.ui.screens;
import carcer.ui.layers;
import sdl2w;
import bmin.string_interop;
#include "macros.h"
#include "../../setupTestUi.h"

namespace {

constexpr const char* TEST_PARTY_TEMPLATE_NAMES[] = {
    "testPartyMember1",
    "testPartyMember2",
    "testPartyMember3",
    "testPartyMember4",
    "testPartyMember5",
    "testPartyMember6",
};

const bmin::DynArray<bmin::DynArray<bmin::String>> PARTY_MEMBER_ITEMS = {
    {"PotionHealing", "DaggerBronze", "DaggerBronze", "DaggerBronze", "SwordBronze"},
    {"ShortSwordBronze", "SwordBronze"},
    {"LongbowOak",
     "ArrowsStone",
     "ArrowsStone",
     "ArrowsStone",
     "ArrowsStone",
     "PotionHealing",
     "DaggerBronze"},
    {"ShirtSimple0", "PantsSimple0", "DaggerBronze", "DaggerBronze"},
    {"GlovesLeather", "HatLeather", "ShortSwordBronze", "SwordBronze"},
    {"BootsLeather", "NecklaceSilver", "DaggerBronze"},
};

void setupTestParty(model::Player& player, db::Database& database) {
  player.party.clear();
  player.currentPartyMemberIndex = 0;
  player.currentPartyMemberInventoryIndex = 0;
  player.gold = 1234;

  for (size_t i = 0; i < PARTY_MEMBER_ITEMS.size(); ++i) {
    model::CharacterPlayer member(
        database.getCharacterTemplate(TEST_PARTY_TEMPLATE_NAMES[i]));

    for (const auto& itemName : PARTY_MEMBER_ITEMS[i]) {
      model::characterPlayerAddItemToInventory(
          member, database.getItemTemplate(bmin::toStringView(itemName)), 1);
    }

    player.party.pushBack(std::move(member));
  }
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start LayerInventory integration test" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  setupTestParty(stateManager.getState().player, database);

  bmin::UniquePtr<layers::LayerManager> layerManager;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "LayerInventory test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    auto* layerInventory = layers::createInventoryLayer(&window);
    layerManager->addLayer(layerInventory);

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
    draw.setBackgroundColor({100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{640, 480, "LayerInventory Test"},
              _init,
              _updateRender,
              [&]() { layerManager.reset(); });

  LOG(INFO) << "End LayerInventory integration test" << LOG_ENDL;
  return 0;
}
