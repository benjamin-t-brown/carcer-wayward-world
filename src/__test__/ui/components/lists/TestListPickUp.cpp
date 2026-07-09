#include "../../../setupTestUi.h"
#include "db/Database.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/components/lists/ListPickUp.h"
#include <memory>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"
#include "bmin/StringInterop.h"

namespace {

ui::ListPickUpProps buildListPickUpPropsFromDatabase(db::Database& database) {
  ui::ListPickUpProps listProps;
  listProps.width = 500;
  const bmin::DynArray<bmin::String> itemNames = {
      "PotionHealing",
      "DaggerBronze",
      "ShortSwordBronze",
      "SwordBronze",
      "LongbowOak",
  };

  for (const auto& itemName : itemNames) {
    const auto& itemTemplate = database.getItemTemplate(bmin::toStringView(itemName));
    listProps.items.pushBack({
        .item =
            model::ItemInstance{
                .id = model::createRandomId(),
                .itemTemplateName = itemTemplate.name,
                .quantity = 1,
            },
        .weight = itemTemplate.weight,
        .itemSprite = itemTemplate.iconSpriteName,
    });
  }

  return listProps;
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start ListPickUp test" << LOG_ENDL;
  srand(time(NULL));

  db::Database database;
  database.load();

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ListPickUp test initialized" << LOG_ENDL;

    auto listPickUp = new ui::ListPickUp(&window);
    listPickUp->setId("listPickUp");
    listPickUp->setPos(100, 50);
    listPickUp->setScale(1.f);

    listPickUp->setProps(buildListPickUpPropsFromDatabase(database));

    auto [listWidth, listHeight] = listPickUp->getDims();
    LOG(INFO) << "ListPickUp dimensions: " << listWidth << ", " << listHeight << LOG_ENDL;

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(listPickUp));

    auto& events = window.getEvents();
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                         [&](int x, int y, int button) {
                           LOG(INFO) << "Mouse down at: " << x << ", " << y
                                     << " - button: " << button << LOG_ENDL;
                           for (auto& element : elements) {
                             element->checkMouseDownEvent(x, y, button);
                           }
                         });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      for (auto& element : elements) {
        element->checkMouseUpEvent(x, y, button);
      }
    });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& element : elements) {
      element->checkHoverEvent(window.getEvents().mouseX, window.getEvents().mouseY);
    }

    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();

    for (auto& element : elements) {
      element->render(window.getDeltaTime());
    }
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "ListPickUp Test"}, _init, _updateRender, [&]() {
        elements.clear();
      });
  LOG(INFO) << "End ListPickUp test" << LOG_ENDL;
  return 0;
}
