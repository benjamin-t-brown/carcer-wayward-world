#pragma once

namespace sdl2w {
class Window;
}

namespace db {
class Database;
}

namespace state {
struct State;
class StateManager;
}

namespace ui {

// World-command helpers: game-rule decisions and action dispatch that used to
// live on LayerWorld as static methods. UI-free — they read State and enqueue
// actions, so they stay testable in isolation from the layer.

// True when the player controls the active combat character and combat is
// waiting for that character's action.
bool canPlayerIssueCombatMove(const state::State& state);

// Enqueue a movement in the current turn context (combat move, or a plain world
// move outside combat). No-op while town enemy AI is resolving or when the
// player may not act.
void enqueueMapMove(state::StateManager& stateManager, int dx, int dy);

// Enqueue a combat WAIT for the active party character, if it may act.
void enqueueCombatWait(state::StateManager& stateManager);

// Ensure uiState.selectedPartyMemberId points at a live party member (or is
// cleared when the party is empty). UI selection only.
void ensureCurrentPartyMemberSelection(state::State& state);

// Resolve an EXAMINE / TALK / SPELL aim at the given tile: validates mode,
// range and pending spell, then enqueues the corresponding action.
void confirmWorldActionAim(state::StateManager& stateManager,
                           sdl2w::Window* window,
                           db::Database* database,
                           int tileX,
                           int tileY);

} // namespace ui
