module;
#include <utility>
#include <cstddef>
#include <cstdlib>

export module carcer.actions:combat;
export import carcer.state;
export import carcer.game.map;
import sdl2w;
import :general;
import bmin.string_interop;
import carcer.model.templates;
import carcer.game.combat;
#include "macros.h"

export {

namespace state {

namespace actions {

// Combat actions enqueue follow-up work through StateManager while executing.
class CombatAction : public AbstractAction {
protected:
  // void insertAction(AbstractAction* action, int ms = 0) {
  //   auto* stateManager = getStateManager();
  //   if (stateManager == nullptr) {
  //     return;
  //   }
  //   stateManager->insertAction(stateManager->getActionData(), action, ms);
  // }

  // void enqueueAction(AbstractAction* action, int ms = 0) {
  //   auto* stateManager = getStateManager();
  //   if (stateManager == nullptr) {
  //     return;
  //   }
  //   stateManager->enqueueAction(stateManager->getActionData(), action, ms);
  // }
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class CharacterSetSpriteIndexOffset : public CombatAction {
  bmin::String characterId;
  int offset = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch;
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    character->spriteIndexOffset = offset;
  }

public:
  CharacterSetSpriteIndexOffset(bmin::String _characterId, int _offset)
      : characterId(std::move(_characterId)), offset(_offset) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class EndCombat : public CombatAction {
  // body in combat.cpp: needs :world (WorldSetCamera), impl-only.
  void act() override;

public:
  EndCombat() = default;
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class ModifyAP : public CombatAction {
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch;
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    character->currentAp += delta;
  }

public:
  ModifyAP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class ModifyHP : public CombatAction {
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch;
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    const auto hp = model::getCharacterHp(state->player, *character) + delta;
    model::setCharacterHp(state->player, *character, hp);
  }

public:
  ModifyHP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class MoveCharacter : public CombatAction {
  bmin::String characterId;
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }

    const auto destX = character->x + dx;
    const auto destY = character->y + dy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      return;
    }
    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      return;
    }
    if (orch.findCharacterAt(destX, destY, characterId) != nullptr) {
      return;
    }

    character->x = destX;
    character->y = destY;
    model::updateCharacterFacingFromMove(*character, dx, dy);

    if (model::isPartyMember(state->player, character->id)) {
      game::updateActiveMapVisibilityFromParty(world, state->player, *database);
    }
  }

public:
  MoveCharacter(bmin::String _characterId, int _dx, int _dy)
      : characterId(std::move(_characterId)), dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class RemoveCharacterFromMap : public CombatAction {
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto& characters = state->world.activeMap.characters;
    for (size_t i = 0; i < characters.size();) {
      if (characters[i].id == characterId) {
        if (model::isCharacterEnemy(characters[i])) {
          game::markMapCharacterDefeated(*state, characters[i]);
        }
        characters.erase(i);
        if (state->world.combat.active) {
          model::removeCharacterFromCombatTurnOrder(state->world.combat, characterId);
        }
        return;
      }
      i++;
    }
  }

public:
  explicit RemoveCharacterFromMap(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class DoCPUCombatTurn;

class SetActiveCombatCharacter : public CombatAction {
  bmin::String characterId;

  // body in combat.cpp: needs :world (WorldSetCamera), impl-only.
  void act() override;

public:
  explicit SetActiveCombatCharacter(bmin::String _characterId = bmin::String{})
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class PerformMeleeAttack : public CombatAction {
  bmin::String attackerId;
  bmin::String victimId;

  // body in combat.cpp: needs :world (WorldSpawnDamageParticle), impl-only.
  void act() override;

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class PerformSpellCast : public CombatAction {
  bmin::String casterId;
  bmin::String spellId;
  model::SpellTargetInfo spellTargetInfo;

  // body in combat.cpp: needs :world (WorldSpawnProjectile/DamageParticle), impl-only.
  void doZoneSpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   game::ActiveMapOrchestrator& orch);

  bool verifySpellCanBeCast(const model::CharacterInstance& caster,
                            const model::AbilityTemplate& ability) {
    if (ability.costType == model::AbilityCostType::ABILITY_COST_MANA) {
      if (caster.currentMp < ability.costValue) {
        return false;
      }
    }

    return true;
  }

  // body in combat.cpp: needs :world (WorldSetActionMode), impl-only.
  void act() override;

public:
  explicit PerformSpellCast(const bmin::String& casterId,
                            const bmin::String& spellId,
                            const model::SpellTargetInfo& spellTargetInfo)
      : casterId(casterId), spellId(spellId), spellTargetInfo(spellTargetInfo) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class PerformCharacterDefeated : public CombatAction {
  bmin::String characterId;

  void act() override {
    if (state) {
      game::ActiveMapOrchestrator orch;
      if (!state->world.activeMap.gridId.empty()) {
        orch.fetchMapGrid(state->world.activeMap.gridId);
      }
      if (auto* character = orch.findCharacterById(characterId)) {
        auto* map = orch.getMapInstanceAt(character->x, character->y);
        const auto local =
            orch.activeMapCoordToInstanceCoord(character->x, character->y);
        if (map && local.valid) {
          map->tileLayerNumber = state->world.activeMap.mapLayer;
          game::addTileFieldAt(*map, local.x, local.y, game::TileFieldType::BLOOD);
        }
      }
    }
    insertAction(new PlaySound("yell1"), 0);
    insertAction(new RemoveCharacterFromMap(characterId), 300);
  }

public:
  explicit PerformCharacterDefeated(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class GoNextCombatTurn : public CombatAction {
  void startNewCombatRound() {
    LOG(INFO) << "GoNextCombatTurn: new combat round, resetting AP" << LOG_ENDL;
    state->world.combat.activeTurnIndex = 0;
    game::onNewCombatRound(*state);
  }

  void act() override {
    if (!state) {
      return;
    }

    auto& combat = state->world.combat;
    if (!combat.active || combat.turnOrderIds.empty()) {
      return;
    }

    LOG(INFO) << "GoNextCombatTurn: advancing from turn index " << combat.activeTurnIndex
              << LOG_ENDL;

    combat.activeTurnIndex += 1;
    if (combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
      startNewCombatRound();
    }

    game::ActiveMapOrchestrator orch;
    const auto turnCount = static_cast<int>(combat.turnOrderIds.size());
    for (int attempt = 0; attempt < turnCount; attempt++) {
      const auto index = combat.activeTurnIndex;
      if (index < 0 || index >= turnCount) {
        break;
      }
      const auto& nextId = combat.turnOrderIds[static_cast<size_t>(index)];
      auto* nextCharacter = orch.findCharacterById(nextId);
      if (nextCharacter == nullptr) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          startNewCombatRound();
        }
        continue;
      }
      if (model::isCharacterDefeated(state->player, *nextCharacter)) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          startNewCombatRound();
        }
        continue;
      }
      insertAction(new SetActiveCombatCharacter(nextId), 0);
      LOG(INFO) << "GoNextCombatTurn: next actor is "
                << model::formatCharacterLogLabel(state->world.activeMap, nextId)
                << LOG_ENDL;
      return;
    }
  }

public:
  GoNextCombatTurn() = default;
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class StartCombat : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    LOG(INFO) << "StartCombat: starting combat on grid " << world.activeMap.gridId
              << LOG_ENDL;
    state->turnMode = model::TurnMode::TURN_COMBAT;
    model::addPartyMembersToCombatMap(world, state->player, *database);
    game::updateActiveMapVisibilityFromParty(world, state->player, *database);
    world.combat = model::createCombatFromWorld(world, state->player);
    model::resetAllCombatAp(world, model::COMBAT_STARTING_AP);

    if (world.combat.turnOrderIds.empty()) {
      world.combat.active = false;
      LOG(WARN) << "StartCombat: no combatants found, aborting" << LOG_ENDL;
      return;
    }

    world.combat.activeTurnIndex = 0;
    LOG(INFO) << "StartCombat: turn order has " << world.combat.turnOrderIds.size()
              << " characters" << LOG_ENDL;
    insertAction(new SetActiveCombatCharacter(), 0);
  }

public:
  StartCombat() = default;
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class DoCombatActionCompletion : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    auto& combat = world.combat;

    LOG(INFO) << "DoCombatActionCompletion: checking results for "
              << model::formatCharacterLogLabel(world.activeMap, combat.activeCharacterId)
              << LOG_ENDL;

    bmin::DynArray<bmin::String> defeatedIds;
    for (const auto& character : world.activeMap.characters) {
      if (model::isCharacterDefeated(state->player, character)) {
        defeatedIds.pushBack(character.id);
      }
    }
    for (const auto& id : defeatedIds) {
      insertAction(new PerformCharacterDefeated(id), 0);
    }

    game::ActiveMapOrchestrator orch;
    auto* activeCharacter = orch.findCharacterById(combat.activeCharacterId);
    const auto apRemaining = activeCharacter != nullptr ? activeCharacter->currentAp : 0;
    const auto turnEnded = apRemaining <= 0;

    if (turnEnded) {
      LOG(INFO) << "DoCombatActionCompletion: turn ended, advancing to next character"
                << LOG_ENDL;
      insertAction(new GoNextCombatTurn(), 0);
    } else if (activeCharacter != nullptr) {
      LOG(INFO) << "DoCombatActionCompletion: "
                << model::formatCharacterLogLabel(world.activeMap, combat.activeCharacterId)
                << " has " << apRemaining << " AP remaining, waiting for next action"
                << LOG_ENDL;
      insertAction(new SetActiveCombatCharacter(combat.activeCharacterId), 0);
    }
  }

public:
  DoCombatActionCompletion() = default;
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

struct CombatActionContext {
  bmin::String targetChId;
  bmin::String abilityId;
  model::TileXY targetLoc{};
};

class DoCombatAction : public CombatAction {
  bmin::String chId;
  model::CombatActionType actionType = model::CombatActionType::WAIT;
  CombatActionContext ctx;

  void handleMove() {
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    const auto& actorId = world.combat.activeCharacterId;
    game::ActiveMapOrchestrator orch;
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      return;
    }
    auto dx = ctx.targetLoc.x;
    auto dy = ctx.targetLoc.y;

    model::updateCharacterFacingFromMove(*actor, dx, dy);

    const auto destX = actor->x + dx;
    const auto destY = actor->y + dy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      return;
    }

    if (auto* occupant = orch.findCharacterAt(destX, destY, actorId)) {
      const auto actorIsEnemy = model::isCharacterEnemy(*actor);
      const auto occupantIsEnemy = model::isCharacterEnemy(*occupant);
      if (actorIsEnemy != occupantIsEnemy) {
        insertAction(new PerformMeleeAttack(actorId, occupant->id), 0);
        insertAction(new ModifyAP(actorId, -model::COMBAT_ATTACK_COST), 0);
        return;
      }
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      return;
    }

    insertAction(new MoveCharacter(actorId, ctx.targetLoc.x, ctx.targetLoc.y), 0);
    insertAction(new ModifyAP(actorId, -model::COMBAT_MOVE_COST), 0);
  }

  void handleSpell() {
    model::SpellTargetInfo spellTargetInfo;
    spellTargetInfo.targetCharacterId = ctx.targetChId;
    spellTargetInfo.tileX = ctx.targetLoc.x;
    spellTargetInfo.tileY = ctx.targetLoc.y;
    if (ctx.abilityId.empty()) {
      LOG(INFO) << "DoCombatAction: SPELL with empty spellId" << LOG_ENDL;
      return;
    }
    insertAction(new PerformSpellCast(chId, ctx.abilityId, spellTargetInfo), 0);
    insertAction(nullptr, 150);
  }

  void act() override {
    if (!state || !state->world.combat.active) {
      return;
    }

    const char* actionLabel = "?";
    switch (actionType) {
    case model::CombatActionType::MOVE:
      actionLabel = "MOVE";
      break;
    case model::CombatActionType::SHOOT:
      actionLabel = "SHOOT";
      break;
    case model::CombatActionType::SPELL:
      actionLabel = "SPELL";
      break;
    case model::CombatActionType::WAIT:
      actionLabel = "WAIT";
      break;
    }
    if (actionType == model::CombatActionType::MOVE) {
      LOG(INFO) << "DoCombatAction: " << actionLabel << " for "
                << model::formatCharacterLogLabel(state->world.activeMap,
                                                  state->world.combat.activeCharacterId)
                << " (" << ctx.targetLoc.x << ", " << ctx.targetLoc.y << ")" << LOG_ENDL;
    } else {
      LOG(INFO) << "DoCombatAction: " << actionLabel << " for "
                << model::formatCharacterLogLabel(state->world.activeMap,
                                                  state->world.combat.activeCharacterId)
                << LOG_ENDL;
    }

    state->world.combat.isWaitingForAction = false;

    switch (actionType) {
    case model::CombatActionType::MOVE:
      handleMove();
      break;
    case model::CombatActionType::SPELL:
      handleSpell();
      break;
    case model::CombatActionType::SHOOT:
      insertAction(new DoCombatActionCompletion(), 0);
      break;
    case model::CombatActionType::WAIT: {
      game::ActiveMapOrchestrator orch;
      orch.fetchMapGrid(state->world.activeMap.gridId);
      auto* character = orch.findCharacterById(chId);
      if (character != nullptr) {
        insertAction(new ModifyAP(chId, -character->currentAp), 0);
      }
      break;
    }
    }
    insertAction(new DoCombatActionCompletion(), 0);
  }

public:
  explicit DoCombatAction(const bmin::String& chId, model::CombatActionType _actionType)
      : chId(chId), actionType(_actionType) {}

  DoCombatAction(const bmin::String& chId,
                 model::CombatActionType _actionType,
                 const CombatActionContext& ctx)
      : chId(chId), actionType(_actionType), ctx(ctx) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class DoCPUCombatTurn : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    const auto& actorId = world.combat.activeCharacterId;

    // Stale CPU turns can land after GoNextCombatTurn advances to a party member.
    // Re-arm waiting so the player can act; do not auto-WAIT away their turn.
    if (model::isPartyMember(state->player, actorId)) {
      world.combat.isWaitingForAction = true;
      return;
    }

    LOG(INFO) << "DoCPUCombatTurn: choosing action for "
              << model::formatCharacterLogLabel(world.activeMap, actorId) << LOG_ENDL;

    game::ActiveMapOrchestrator orch;
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      insertAction(nullptr, 300);
      insertAction(new DoCombatAction(actorId, model::CombatActionType::WAIT), 0);
      return;
    }

    if (actor->combatBehaviorCombat == model::CombatBehaviorName::SEEK_AND_MELEE) {
      auto dx = 0;
      auto dy = 0;
      if (game::chooseSeekAndMeleeCombatAction(
              world, state->player, *actor, *database, dx, dy)) {
        insertAction(nullptr, 300);
        insertAction(new DoCombatAction(
                         actorId, model::CombatActionType::MOVE, {.targetLoc = {dx, dy}}),
                     0);
        return;
      }
    }

    insertAction(nullptr, 300);
    insertAction(new DoCombatAction(actorId, model::CombatActionType::WAIT), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state

} // export
