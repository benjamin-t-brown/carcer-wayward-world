export module carcer.actions.world;
import sdl2w;

export import :CharacterSetSpriteIndexOffset;
export import :WorldExamineAt;
export import :WorldInteractAt;
export import :WorldLoadActiveMap;
export import :WorldMoveActionAim;
export import :WorldSetActionAim;
export import :WorldSetActionMode;
export import :WorldSetCamera;
export import :WorldSetCameraMode;
export import :WorldSpawnDamageParticle;
export import :WorldSpawnPlayerAtMarker;
export import :WorldSpawnPlayerAtXY;
export import :WorldSpawnProjectile;
export import :WorldTalkAt;
export import :ModifyPartyMemberHp;
export import :PerformTownMeleeAttack;
export import :WorldSpawnPlayer;
export import :WorldTravel;
export import :ClearTownEnemyAiResolving;
export import :TownEnemySeekAndMelee;
export import :TownEnemyAiAfterPlayerMove;
export import :WorldMovePlayer;

export namespace state {

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt);

inline void worldUpdate(StateManager& stateManager, int dt) {
  worldUpdate(nullptr, stateManager, dt);
}

void worldProcessPendingTriggers(sdl2w::Window* window,
                                 StateManager& stateManager);

} // namespace state
