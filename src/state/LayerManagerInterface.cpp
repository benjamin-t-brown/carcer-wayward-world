module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>

module carcer.state;

namespace state {

void* LayerManagerInterface::layerManager = nullptr;
LayerManagerOps LayerManagerInterface::ops{};

void LayerManagerInterface::setLayerManager(void* _layerManager) {
  layerManager = _layerManager;
}

void* LayerManagerInterface::getLayerManager() { return layerManager; }

void LayerManagerInterface::bindLayerOps(LayerManagerOps _ops) { ops = _ops; }

void LayerManagerInterface::closeLayer(LayerId id) {
  if (layerManager && ops.closeById) {
    ops.closeById(layerManager, id);
  }
}

void LayerManagerInterface::closeLayer(std::string_view id) {
  if (layerManager && ops.closeByString) {
    ops.closeByString(layerManager, id);
  }
}

void LayerManagerInterface::showInventory(void* window) {
  if (layerManager && ops.showInventory) {
    ops.showInventory(layerManager, window);
  }
}

void LayerManagerInterface::showMagic(void* window) {
  if (layerManager && ops.showMagic) {
    ops.showMagic(layerManager, window);
  }
}

void LayerManagerInterface::showSpellCast(void* window, const bmin::String& chId) {
  if (layerManager && ops.showSpellCast) {
    ops.showSpellCast(layerManager, window, &chId);
  }
}

void LayerManagerInterface::showSpellInfo(void* window, const bmin::String& spellName) {
  if (layerManager && ops.showSpellInfo) {
    ops.showSpellInfo(layerManager, window, &spellName);
  }
}

void LayerManagerInterface::showEquipRunes(void* window,
                                           const bmin::String& characterPlayerId) {
  if (layerManager && ops.showEquipRunes) {
    ops.showEquipRunes(layerManager, window, &characterPlayerId);
  }
}

void LayerManagerInterface::showPickUp(void* window) {
  if (layerManager && ops.showPickUp) {
    ops.showPickUp(layerManager, window, nullptr, nullptr);
  }
}

void LayerManagerInterface::showPickUp(void* window, int containerX, int containerY) {
  if (layerManager && ops.showPickUp) {
    ops.showPickUp(layerManager, window, &containerX, &containerY);
  }
}

void LayerManagerInterface::showPickupContext(void* window,
                                              const model::ItemInstance& item) {
  if (layerManager && ops.showPickupContext) {
    ops.showPickupContext(layerManager, window, &item);
  }
}

void LayerManagerInterface::showInventoryContext(void* window,
                                                 const bmin::String& itemId,
                                                 const bmin::String& itemName) {
  if (layerManager && ops.showInventoryContext) {
    ops.showInventoryContext(layerManager, window, &itemId, &itemName);
  }
}

void LayerManagerInterface::showDropConfirm(void* window,
                                            const bmin::String& characterPlayerId,
                                            const bmin::String& itemId) {
  if (layerManager && ops.showDropConfirm) {
    ops.showDropConfirm(layerManager, window, &characterPlayerId, &itemId);
  }
}

void LayerManagerInterface::showGiveContext(void* window,
                                            const bmin::String& fromCharacterPlayerId,
                                            const bmin::String& itemId) {
  if (layerManager && ops.showGiveContext) {
    ops.showGiveContext(layerManager, window, &fromCharacterPlayerId, &itemId);
  }
}

void LayerManagerInterface::showPopupText(void* window,
                                          const bmin::String& title,
                                          const bmin::String& text) {
  if (layerManager && ops.showPopupText) {
    ops.showPopupText(layerManager, window, &title, &text);
  }
}

void LayerManagerInterface::showSpecialEvent(void* window,
                                             const bmin::String& eventId,
                                             State& state) {
  if (layerManager && ops.showSpecialEvent) {
    ops.showSpecialEvent(layerManager, window, &eventId, &state);
  }
}

void LayerManagerInterface::cancelEquipRunes() {
  if (layerManager && ops.cancelEquipRunes) {
    ops.cancelEquipRunes(layerManager);
  }
}

} // namespace state
