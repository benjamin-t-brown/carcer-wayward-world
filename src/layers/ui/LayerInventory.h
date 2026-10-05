#pragma once

#include "../UiLayer.h"
#include "ui/pages/PageInventory.h"

namespace db {
class Database;
}

namespace layers {

class LayerInventory : public UiLayer {
private:
  static void populateSpecialItemsFromStorage(
      const bmin::Map<bmin::String, bmin::String>& storage,
      db::Database& database,
      bmin::DynArray<ui::PageInventorySpecialItem>& out);

public:
  constexpr static std::string_view LAYER_ID = "layer_inventory";

  explicit LayerInventory(sdl2w::Window* _window);
  virtual ~LayerInventory() = default;

  void onKeyDown(std::string_view key, int keyCode) override;

  void syncInventoryPartyMember();
};

} // namespace layers
