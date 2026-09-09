#pragma once

#include "bmin/DynArray.h"
#include "model/templates/UtilityTypes.h"
#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"
#include "state/WorldActions.hpp"

namespace ui {
class InGameLayout;
}

namespace layers {

class UiLayer;

/**
 * Pushes world State into LayerWorld's UI (InGameLayout, title bar, action
 * buttons, party list) and wires the observers those widgets need.
 *
 * State and database are reached through the inherited singleton interfaces —
 * the same instances LayerWorld sees. The per-layer handles that are not
 * singletons (owned UI elements, window, layer manager) come from `owner`,
 * whose getUiElement/getWindow/getLayerManager accessors are public.
 */
class WorldViewSync : public state::StateManagerInterface,
                      public state::DatabaseInterface {
public:
  explicit WorldViewSync(UiLayer& owner) : owner(owner) {}

  // Full state -> UI pass (party list, action types, title bar, observers,
  // highlights). Camera alignment stays on the layer.
  void refresh();

  // Cheap per-frame refreshes, also callable independently of a full refresh().
  void syncWorldActionModeHighlight();
  void syncActionModeCancelButton();
  void syncCombatTitleBar();

private:
  bool assertInterfaces() const;
  void attachWorldActionObservers(ui::InGameLayout* inGameLayout);
  void attachPartyMemberObservers(ui::InGameLayout* inGameLayout);
  static void setWorldActionTypes(model::TurnMode turnMode,
                                  bmin::DynArray<state::WorldActionType>& dest);

  UiLayer& owner;
};

} // namespace layers
