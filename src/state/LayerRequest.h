#pragma once

#include "bmin/String.h"
#include <optional>
#include <string_view>

namespace state {

struct State;

enum class LayerId {
  World,
  Inventory,
  InventoryContext,
  Magic,
  SpellCast,
  SpellInfo,
  EquipRunes,
  PickUp,
  PickUpContext,
  DropConfirm,
  GiveContext,
  PopupText,
  SpecialEvent,
};

inline std::string_view layerIdString(LayerId id) {
  switch (id) {
  case LayerId::World: return "layer_world";
  case LayerId::Inventory: return "layer_inventory";
  case LayerId::InventoryContext: return "layer_inventory_context";
  case LayerId::Magic: return "layer_magic";
  case LayerId::SpellCast: return "layer_spell_cast";
  case LayerId::SpellInfo: return "layer_spell_info";
  case LayerId::EquipRunes: return "layer_equip_runes";
  case LayerId::PickUp: return "layer_pick_up";
  case LayerId::PickUpContext: return "layer_pick_up_context";
  case LayerId::DropConfirm: return "layer_drop_confirm";
  case LayerId::GiveContext: return "layer_give_context";
  case LayerId::PopupText: return "layer_popup_text";
  case LayerId::SpecialEvent: return "layer_special_event";
  }
  return {};
}

inline std::optional<LayerId> layerIdFromString(std::string_view value) {
  for (auto id : {LayerId::World,
                  LayerId::Inventory,
                  LayerId::InventoryContext,
                  LayerId::Magic,
                  LayerId::SpellCast,
                  LayerId::SpellInfo,
                  LayerId::EquipRunes,
                  LayerId::PickUp,
                  LayerId::PickUpContext,
                  LayerId::DropConfirm,
                  LayerId::GiveContext,
                  LayerId::PopupText,
                  LayerId::SpecialEvent}) {
    if (layerIdString(id) == value) {
      return id;
    }
  }
  return std::nullopt;
}

struct LayerRequest {
  LayerId id;
  bmin::String a;
  bmin::String b;
  int x = 0;
  int y = 0;
  bool hasPosition = false;
};

void pushLayerRequest(State& state, LayerRequest request);
void removeLayerRequest(State& state, LayerId id);

} // namespace state
