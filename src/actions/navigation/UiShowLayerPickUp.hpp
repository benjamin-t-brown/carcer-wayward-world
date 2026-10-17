#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "game/map/MapPickup.h"
#include "game/map/TileTriggers.h"
#include "model/templates/UtilityTypes.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include <optional>

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPickUp; }
  std::optional<std::pair<int, int>> containerTile;

  void pushNoItemsNearbyWarning() {
    UiFloatingNotification notification;
    notification.id = model::createRandomId();
    notification.message = TRANSLATE("Get: No items nearby.");
    notification.type = UiFloatingNotificationType::WARNING;
    model::timerStructStart(notification.timer,
                            state->settings.floatingNotificationDurationMs);
    state->uiState.floatingNotifications.pushBack(std::move(notification));
    ++state->uiState.floatingNotificationRevision;
    LOG(INFO) << "UiShowLayerPickUp: Get: No items nearby." << LOG_ENDL;
  }

  void act() override {
    if (!state) {
      return;
    }

    // World Get button (no container): warn instead of opening an empty pick-up UI.
    if (!containerTile) {
      auto* database = getDatabase();
      if (!database) {
        LOG(WARN) << "UiShowLayerPickUp::act: database is nullptr" << LOG_ENDL;
        return;
      }
      const auto* avatar = game::findPartyAvatarOnActiveMap(state->world.activeMap,
                                                           state->player);
      auto nearby = bmin::DynArray<model::ItemInstance>{};
      if (avatar != nullptr) {
        nearby = game::collectItemsWithinPickupRange(state->world.activeMap,
                                                    state->mapInstances,
                                                    *avatar,
                                                    game::PICKUP_PATH_RANGE,
                                                    *database);
      }
      if (nearby.empty()) {
        pushNoItemsNearbyWarning();
        return;
      }
    }

    LayerRequest request{.id = LayerId::PickUp};
    if (containerTile) {
      request.x = containerTile->first;
      request.y = containerTile->second;
      request.hasPosition = true;
    }
    pushLayerRequest(*state, std::move(request));
  }

public:
  explicit UiShowLayerPickUp(sdl2w::Window* /*window*/) {}

  UiShowLayerPickUp(sdl2w::Window* /*window*/, int containerX, int containerY)
      : containerTile(std::make_pair(containerX, containerY)) {}
};

} // namespace actions

} // namespace state
