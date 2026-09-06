module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <optional>

export module carcer.ui.helpers;
export import carcer.ui.core;
export import carcer.model;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

// --- from ui/helpers/modalLayoutFit.h ---
namespace ui {


// How ModalStandard / ModalSmall map width/height props to the final rect.
// CappedCentered (default): treat width/height as window dims, then cap + center.
// FullBleed: use width/height as the modal size as-is (no cap/center).
enum class LayoutFit { CappedCentered, FullBleed };

// Landscape max size class. Portrait still uses near-full margins for both.
enum class ModalSizeClass { Standard, Small };

struct LayoutRect {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

// Portrait: near-full with small margins.
// Landscape Standard: ~500×min(500, h−50). Small: ~400×min(420, h−50).
LayoutRect computeCappedCenteredRect(int windowW,
                                     int windowH,
                                     ModalSizeClass sizeClass = ModalSizeClass::Standard);

// After a CappedCentered modal->setProps, sync the host page/minipage style to the
// fitted rect so getPos()+getDims() (often child[0] size) align with drawn controls.
// Call while host style still holds window width/height. Pass the same sizeClass
// the modal used.
void syncHostStyleToCappedCentered(BaseStyle& style,
                                   ModalSizeClass sizeClass = ModalSizeClass::Standard);

} // namespace ui

// --- from ui/helpers/keyboardShortcuts.h ---
namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key, model::TurnMode turnMode);

struct MoveDelta {
  int dx = 0;
  int dy = 0;
};

std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key);

bool isCancelActionKey(std::string_view key);

bool isConfirmActionKey(std::string_view key);

bool isCombatWaitKey(std::string_view key);

/** `m` / `M` — open combat spell-cast list (combat-only at call site). */
bool isOpenSpellCastKey(std::string_view key);

/** `r` / `R` — open magic setup (LayerMagic); always, combat or not. */
bool isOpenMagicSetupKey(std::string_view key);

/** Keys "1"-"6" → party index 0-5. */
std::optional<int> getPartyMemberIndexFromKey(std::string_view key);

/** Keys "a"-"z" / "A"-"Z" → pick-up list index 0-25. */
std::optional<int> getPickUpItemIndexFromKey(std::string_view key);

} // namespace ui

// --- from ui/helpers/worldActions.h ---
namespace ui {

void setHeldMoveActive(state::StateManager& stateManager, bool isActive);
void cancelCurrentWorldActionMode(state::StateManager& stateManager);
void showMagicSetupLayer(state::StateManager& stateManager, sdl2w::Window* window);
void showSpellCastLayer(state::StateManager& stateManager, sdl2w::Window* window);
void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType,
                         sdl2w::Window* window = nullptr);

} // namespace ui

} // export
