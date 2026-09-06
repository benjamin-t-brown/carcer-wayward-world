actions/combat/combat.o: .carcer-bmi/carcer.actions.combat.o .carcer-bmi/carcer.actions.general.o .carcer-bmi/carcer.actions.world_effects.o .carcer-bmi/carcer.data.o .carcer-bmi/carcer.game.combat.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
data/stats.o: .carcer-bmi/carcer.data.o
data/templates.o: .carcer-bmi/carcer.data.o
db/Database.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o
db/loaders/LoadAbilityJson.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadAbilityTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadCharacterTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadItemTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadMapGridTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadMapTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadSpecialEvents.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o .carcer-bmi/carcer.lib.StringUtil.o
db/loaders/LoadSpellTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadStatusEffectTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
db/loaders/LoadTilesetTemplates.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.lib.Json.o
game/combat/CombatRunner.o: .carcer-bmi/carcer.game.combat.o .carcer-bmi/carcer.model.o
game/combat/Damage.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.game.combat.o .carcer-bmi/carcer.model.o
game/combat/EnemyBehavior.o: .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.combat.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/combat/SpellRules.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.combat.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/combat/projectileHelpers.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.game.combat.o
game/diceHelpers.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.game.combat.o
game/map/ActiveMapOrchestrator.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/Camera.o: .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/MapPathfinding.o: .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/MapPersistence.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/MapPickup.o: .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/MapVision.o: .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/MapWalkability.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
game/map/TileDistance.o: .carcer-bmi/carcer.game.map.o
game/map/TileFields.o: .carcer-bmi/carcer.game.map.TileFields.o
game/map/TileTriggers.o: .carcer-bmi/carcer.db.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o
in3/ConditionEvaluator.o: .carcer-bmi/carcer.in3.o
in3/EventRunnerHelpers.o: .carcer-bmi/carcer.in3.o .carcer-bmi/carcer.lib.StringUtil.o
in3/SpecialEventRunner.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.in3.o
in3/StringEvaluator.o: .carcer-bmi/carcer.in3.o
layers/LayerManager.o: .carcer-bmi/carcer.layers.o
lib/Json.o: .carcer-bmi/carcer.lib.Json.o
lib/hiscore/hiscore.o: .carcer-bmi/carcer.lib.StringUtil.o .carcer-bmi/carcer.lib.hiscore.hiscore.o
main.o: .carcer-bmi/carcer.o
model/characters.o: .carcer-bmi/carcer.model.o
model/combat.o: .carcer-bmi/carcer.model.o
model/maps.o: .carcer-bmi/carcer.model.o
model/world.o: .carcer-bmi/carcer.model.o
state/ActionBus.o: .carcer-bmi/carcer.state.o
state/DatabaseInterface.o: .carcer-bmi/carcer.db.o .carcer-bmi/carcer.state.o
state/LayerManagerInterface.o: .carcer-bmi/carcer.state.o
state/StateManager.o: .carcer-bmi/carcer.state.o
state/StateManagerInterface.o: .carcer-bmi/carcer.state.o
state/UiManager.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.state.o
state/WorldUpdater.o: .carcer-bmi/carcer.actions.o .carcer-bmi/carcer.data.o .carcer-bmi/carcer.game.map.o .carcer-bmi/carcer.model.o .carcer-bmi/carcer.ui.helpers.o .carcer-bmi/carcer.world_updater.o
ui/FontScale.o: .carcer-bmi/carcer.ui.core.o
ui/KeyboardHeldScroll.o: .carcer-bmi/carcer.data.o .carcer-bmi/carcer.ui.KeyboardHeldScroll.o
ui/UiElement.o: .carcer-bmi/carcer.state.o .carcer-bmi/carcer.ui.core.o
ui/components/ChCompactInfo.o: .carcer-bmi/carcer.ui.components.o .carcer-bmi/carcer.ui.core.o .carcer-bmi/carcer.ui.elements.o
ui/helpers/keyboardShortcuts.o: .carcer-bmi/carcer.model.o .carcer-bmi/carcer.state.o .carcer-bmi/carcer.ui.helpers.o
ui/helpers/modalLayoutFit.o: .carcer-bmi/carcer.ui.core.o .carcer-bmi/carcer.ui.helpers.o
ui/helpers/worldActions.o: .carcer-bmi/carcer.actions.o .carcer-bmi/carcer.model.o .carcer-bmi/carcer.state.o .carcer-bmi/carcer.ui.helpers.o
