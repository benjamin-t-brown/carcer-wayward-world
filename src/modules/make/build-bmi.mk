CXX ?= g++
CARCER_MOD ?= modules
SDL2W_MOD ?= lib/sdl2w/modules
BMIN_MOD ?= lib/sdl2w/modules/bmin
FLAGS = -Wall -std=c++23 -g -fmodules-ts -I$(CARCER_MOD) -I$(SDL2W_MOD) -I$(BMIN_MOD)
OBJDIR = .carcer-bmi

.PHONY: all clean

all: $(OBJDIR)/carcer.o
	@mkdir -p gcm.cache
	@touch gcm.cache/.carcer-ready

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/carcer.game.map.TileFields.o: game/map/TileFields.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c game/map/TileFields.cppm -o $@

$(OBJDIR)/carcer.lib.Json.o: lib/Json.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c lib/Json.cppm -o $@

$(OBJDIR)/carcer.lib.StringUtil.o: lib/StringUtil.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c lib/StringUtil.cppm -o $@

$(OBJDIR)/carcer.lib.hiscore.hiscore.o: lib/hiscore/hiscore.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c lib/hiscore/hiscore.cppm -o $@

$(OBJDIR)/carcer.model.instances-ItemInstance.o: model/instances/ItemInstance.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/ItemInstance.cppm -o $@

$(OBJDIR)/carcer.model.templates-AbilityTypes.o: model/templates/AbilityTypes.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/AbilityTypes.cppm -o $@

$(OBJDIR)/carcer.model.templates-CharacterStatDefinitions.o: model/stats/CharacterStatDefinitions.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/stats/CharacterStatDefinitions.cppm -o $@

$(OBJDIR)/carcer.model.templates-CharacterStats.o: model/stats/CharacterStats.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/stats/CharacterStats.cppm -o $@

$(OBJDIR)/carcer.model.templates-MapGrids.o: model/templates/MapGrids.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/MapGrids.cppm -o $@

$(OBJDIR)/carcer.model.templates-Maps.o: model/templates/Maps.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/Maps.cppm -o $@

$(OBJDIR)/carcer.model.templates-RuneTypes.o: model/templates/RuneTypes.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/RuneTypes.cppm -o $@

$(OBJDIR)/carcer.model.templates-SpecialEvents.o: model/templates/SpecialEvents.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/SpecialEvents.cppm -o $@

$(OBJDIR)/carcer.model.templates-Tileset.o: model/templates/Tileset.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/Tileset.cppm -o $@

$(OBJDIR)/carcer.model.templates-UtilityTypes.o: model/templates/UtilityTypes.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/UtilityTypes.cppm -o $@

$(OBJDIR)/carcer.ui.core-FontScale.o: ui/FontScale.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/FontScale.cppm -o $@

$(OBJDIR)/carcer.ui.core-SdlPixels.o: ui/SdlPixels.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/SdlPixels.cppm -o $@

$(OBJDIR)/carcer.model.templates-Abilities.o: model/templates/Abilities.cppm $(OBJDIR)/carcer.model.templates-AbilityTypes.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/Abilities.cppm -o $@

$(OBJDIR)/carcer.model.templates-StatusEffects.o: model/templates/StatusEffects.cppm $(OBJDIR)/carcer.model.templates-AbilityTypes.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/StatusEffects.cppm -o $@

$(OBJDIR)/carcer.model.templates-CharacterDerivedStats.o: model/stats/CharacterDerivedStats.cppm $(OBJDIR)/carcer.model.templates-CharacterStats.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/stats/CharacterDerivedStats.cppm -o $@

$(OBJDIR)/carcer.model.templates-CharacterTemplate.o: model/templates/CharacterTemplate.cppm $(OBJDIR)/carcer.model.templates-CharacterStats.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/CharacterTemplate.cppm -o $@

$(OBJDIR)/carcer.model.templates-Items.o: model/templates/Items.cppm $(OBJDIR)/carcer.model.templates-AbilityTypes.o $(OBJDIR)/carcer.model.templates-RuneTypes.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/Items.cppm -o $@

$(OBJDIR)/carcer.model.templates-Spells.o: model/templates/Spells.cppm $(OBJDIR)/carcer.model.templates-RuneTypes.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/Spells.cppm -o $@

$(OBJDIR)/carcer.ui.core-colors.o: ui/colors.cppm $(OBJDIR)/carcer.ui.core-SdlPixels.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/colors.cppm -o $@

$(OBJDIR)/carcer.model.templates-CharacterDerivedStatDefinitions.o: model/stats/CharacterDerivedStatDefinitions.cppm $(OBJDIR)/carcer.model.templates-CharacterDerivedStats.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/stats/CharacterDerivedStatDefinitions.cppm -o $@

$(OBJDIR)/carcer.ui.core-TextStyle.o: ui/TextStyle.cppm $(OBJDIR)/carcer.ui.core-SdlPixels.o $(OBJDIR)/carcer.ui.core-colors.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/TextStyle.cppm -o $@

$(OBJDIR)/carcer.model.templates.o: model/templates/templates.cppm $(OBJDIR)/carcer.model.templates-Abilities.o $(OBJDIR)/carcer.model.templates-AbilityTypes.o $(OBJDIR)/carcer.model.templates-CharacterDerivedStatDefinitions.o $(OBJDIR)/carcer.model.templates-CharacterDerivedStats.o $(OBJDIR)/carcer.model.templates-CharacterStatDefinitions.o $(OBJDIR)/carcer.model.templates-CharacterStats.o $(OBJDIR)/carcer.model.templates-CharacterTemplate.o $(OBJDIR)/carcer.model.templates-Items.o $(OBJDIR)/carcer.model.templates-MapGrids.o $(OBJDIR)/carcer.model.templates-Maps.o $(OBJDIR)/carcer.model.templates-RuneTypes.o $(OBJDIR)/carcer.model.templates-SpecialEvents.o $(OBJDIR)/carcer.model.templates-Spells.o $(OBJDIR)/carcer.model.templates-StatusEffects.o $(OBJDIR)/carcer.model.templates-Tileset.o $(OBJDIR)/carcer.model.templates-UtilityTypes.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/templates/templates.cppm -o $@

$(OBJDIR)/carcer.db.o: db/db.cppm $(OBJDIR)/carcer.lib.Json.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c db/db.cppm -o $@

$(OBJDIR)/carcer.in3.o: in3/in3.cppm $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c in3/in3.cppm -o $@

$(OBJDIR)/carcer.model.instances-TileInstance.o: model/instances/TileInstance.cppm $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/TileInstance.cppm -o $@

$(OBJDIR)/carcer.model.instances-CharacterInstance.o: model/instances/CharacterInstance.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/CharacterInstance.cppm -o $@

$(OBJDIR)/carcer.model.instances-CharacterPlayer.o: model/instances/CharacterPlayer.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.model.instances-ItemInstance.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/CharacterPlayer.cppm -o $@

$(OBJDIR)/carcer.model.instances-MapInstance.o: model/instances/MapInstance.cppm $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.instances-CharacterInstance.o $(OBJDIR)/carcer.model.instances-ItemInstance.o $(OBJDIR)/carcer.model.instances-TileInstance.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/MapInstance.cppm -o $@

$(OBJDIR)/carcer.model.instances-Player.o: model/instances/Player.cppm $(OBJDIR)/carcer.model.instances-CharacterPlayer.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/Player.cppm -o $@

$(OBJDIR)/carcer.model.instances-Combat.o: model/instances/Combat.cppm $(OBJDIR)/carcer.model.instances-CharacterInstance.o $(OBJDIR)/carcer.model.instances-Player.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/Combat.cppm -o $@

$(OBJDIR)/carcer.model.instances-World.o: model/instances/World.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.model.instances-CharacterInstance.o $(OBJDIR)/carcer.model.instances-Combat.o $(OBJDIR)/carcer.model.instances-MapInstance.o $(OBJDIR)/carcer.model.instances-Player.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/World.cppm -o $@

$(OBJDIR)/carcer.model.instances.o: model/instances/instances.cppm $(OBJDIR)/carcer.model.instances-CharacterInstance.o $(OBJDIR)/carcer.model.instances-CharacterPlayer.o $(OBJDIR)/carcer.model.instances-Combat.o $(OBJDIR)/carcer.model.instances-ItemInstance.o $(OBJDIR)/carcer.model.instances-MapInstance.o $(OBJDIR)/carcer.model.instances-Player.o $(OBJDIR)/carcer.model.instances-TileInstance.o $(OBJDIR)/carcer.model.instances-World.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c model/instances/instances.cppm -o $@

$(OBJDIR)/carcer.state.o: state/State.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c state/State.cppm -o $@

$(OBJDIR)/carcer.actions.combat-CombatAction.o: actions/combat/CombatAction.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/CombatAction.cppm -o $@

$(OBJDIR)/carcer.actions.general.o: actions/general/general.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/general/general.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiCancelEquipRunes.o: actions/ui/layers/UiCancelEquipRunes.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiCancelEquipRunes.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiCommitEquipRunes.o: actions/ui/layers/UiCommitEquipRunes.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiCommitEquipRunes.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiRemoveLayer.o: actions/ui/layers/UiRemoveLayer.cppm $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiRemoveLayer.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerDropContext.o: actions/ui/layers/UiShowLayerDropContext.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerDropContext.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerEquipRunes.o: actions/ui/layers/UiShowLayerEquipRunes.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerEquipRunes.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerGiveContext.o: actions/ui/layers/UiShowLayerGiveContext.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerGiveContext.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventory.o: actions/ui/layers/UiShowLayerInventory.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerInventory.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventoryContext.o: actions/ui/layers/UiShowLayerInventoryContext.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerInventoryContext.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerMagic.o: actions/ui/layers/UiShowLayerMagic.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerMagic.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickUp.o: actions/ui/layers/UiShowLayerPickUp.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerPickUp.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickupContext.o: actions/ui/layers/UiShowLayerPickupContext.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerPickupContext.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPopupText.o: actions/ui/layers/UiShowLayerPopupText.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerPopupText.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpecialEvent.o: actions/ui/layers/UiShowLayerSpecialEvent.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerSpecialEvent.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellCast.o: actions/ui/layers/UiShowLayerSpellCast.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerSpellCast.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellInfo.o: actions/ui/layers/UiShowLayerSpellInfo.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/UiShowLayerSpellInfo.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiAdjustEquippedRune.o: actions/ui/UiAdjustEquippedRune.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiAdjustEquippedRune.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiContinueSpecialEvent.o: actions/ui/UiContinueSpecialEvent.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiContinueSpecialEvent.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiGiveInventoryItem.o: actions/ui/UiGiveInventoryItem.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiGiveInventoryItem.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiPickUpItem.o: actions/ui/UiPickUpItem.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiPickUpItem.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiPushFloatingNotification.o: actions/ui/UiPushFloatingNotification.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiPushFloatingNotification.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiRemoveFloatingNotification.o: actions/ui/UiRemoveFloatingNotification.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiRemoveFloatingNotification.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiReorderInventoryItem.o: actions/ui/UiReorderInventoryItem.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiReorderInventoryItem.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSelectSpecialEventChoice.o: actions/ui/UiSelectSpecialEventChoice.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSelectSpecialEventChoice.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMember.o: actions/ui/UiSetCurrentPartyMember.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSetCurrentPartyMember.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberInventory.o: actions/ui/UiSetCurrentPartyMemberInventory.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSetCurrentPartyMemberInventory.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberMagic.o: actions/ui/UiSetCurrentPartyMemberMagic.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSetCurrentPartyMemberMagic.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSetSelectedPartyMemberId.o: actions/ui/UiSetSelectedPartyMemberId.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSetSelectedPartyMemberId.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSetSpellReady.o: actions/ui/UiSetSpellReady.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSetSpellReady.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiToggleEquipInventoryItem.o: actions/ui/UiToggleEquipInventoryItem.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiToggleEquipInventoryItem.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiToggleManaSlotRune.o: actions/ui/UiToggleManaSlotRune.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiToggleManaSlotRune.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiUpdateHeldMove.o: actions/ui/UiUpdateHeldMove.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiUpdateHeldMove.cppm -o $@

$(OBJDIR)/carcer.actions.world-ClearTownEnemyAiResolving.o: actions/world/ClearTownEnemyAiResolving.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/ClearTownEnemyAiResolving.cppm -o $@

$(OBJDIR)/carcer.actions.world-ModifyPartyMemberHp.o: actions/world/ModifyPartyMemberHp.cppm $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/ModifyPartyMemberHp.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSetCamera.o: actions/world/WorldSetCamera.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSetCamera.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSetCameraMode.o: actions/world/WorldSetCameraMode.cppm $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSetCameraMode.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSpawnDamageParticle.o: actions/world/WorldSpawnDamageParticle.cppm $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSpawnDamageParticle.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSpawnProjectile.o: actions/world/WorldSpawnProjectile.cppm $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSpawnProjectile.cppm -o $@

$(OBJDIR)/carcer.game.map.o: game/map/map.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c game/map/map.cppm -o $@

$(OBJDIR)/carcer.ui.core-UiElement.o: ui/UiElement.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core-SdlPixels.o $(OBJDIR)/carcer.ui.core-TextStyle.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/UiElement.cppm -o $@

$(OBJDIR)/carcer.actions.combat-PerformMeleeAttack.o: actions/combat/PerformMeleeAttack.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/PerformMeleeAttack.cppm -o $@

$(OBJDIR)/carcer.actions.ui.layers.o: actions/ui/layers/layers.cppm $(OBJDIR)/carcer.actions.ui.layers-UiCancelEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiCommitEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiRemoveLayer.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerDropContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerGiveContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventory.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventoryContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerMagic.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickUp.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickupContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPopupText.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpecialEvent.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellCast.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellInfo.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/layers/layers.cppm -o $@

$(OBJDIR)/carcer.actions.combat-CharacterSetSpriteIndexOffset.o: actions/combat/CharacterSetSpriteIndexOffset.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/CharacterSetSpriteIndexOffset.cppm -o $@

$(OBJDIR)/carcer.actions.combat-EndCombat.o: actions/combat/EndCombat.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/EndCombat.cppm -o $@

$(OBJDIR)/carcer.actions.combat-ModifyAP.o: actions/combat/ModifyAP.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/ModifyAP.cppm -o $@

$(OBJDIR)/carcer.actions.combat-ModifyHP.o: actions/combat/ModifyHP.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/ModifyHP.cppm -o $@

$(OBJDIR)/carcer.actions.combat-MoveCharacter.o: actions/combat/MoveCharacter.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/MoveCharacter.cppm -o $@

$(OBJDIR)/carcer.actions.combat-PerformSpellCast.o: actions/combat/PerformSpellCast.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/PerformSpellCast.cppm -o $@

$(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o: actions/combat/SetActiveCombatCharacter.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/SetActiveCombatCharacter.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiDropInventoryItem.o: actions/ui/UiDropInventoryItem.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiDropInventoryItem.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldExamineAt.o: actions/world/WorldExamineAt.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldExamineAt.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldInteractAt.o: actions/world/WorldInteractAt.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldInteractAt.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldLoadActiveMap.o: actions/world/WorldLoadActiveMap.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldLoadActiveMap.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldMoveActionAim.o: actions/world/WorldMoveActionAim.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldMoveActionAim.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSetActionAim.o: actions/world/WorldSetActionAim.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSetActionAim.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSetActionMode.o: actions/world/WorldSetActionMode.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSetActionMode.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtMarker.o: actions/world/WorldSpawnPlayerAtMarker.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSpawnPlayerAtMarker.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtXY.o: actions/world/WorldSpawnPlayerAtXY.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSpawnPlayerAtXY.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldTalkAt.o: actions/world/WorldTalkAt.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldTalkAt.cppm -o $@

$(OBJDIR)/carcer.game.combat.o: game/combat/combat.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c game/combat/combat.cppm -o $@

$(OBJDIR)/carcer.ui.core-uiUtils.o: ui/uiUtils.cppm $(OBJDIR)/carcer.ui.core-SdlPixels.o $(OBJDIR)/carcer.ui.core-UiElement.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/uiUtils.cppm -o $@

$(OBJDIR)/carcer.actions.ui-UiSelectSpellCast.o: actions/ui/UiSelectSpellCast.cppm $(OBJDIR)/carcer.actions.ui.layers.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/UiSelectSpellCast.cppm -o $@

$(OBJDIR)/carcer.actions.combat-StartCombat.o: actions/combat/StartCombat.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/StartCombat.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldSpawnPlayer.o: actions/world/WorldSpawnPlayer.cppm $(OBJDIR)/carcer.actions.world-WorldLoadActiveMap.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtMarker.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldSpawnPlayer.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldTravel.o: actions/world/WorldTravel.cppm $(OBJDIR)/carcer.actions.world-WorldLoadActiveMap.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtXY.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldTravel.cppm -o $@

$(OBJDIR)/carcer.actions.combat-GoNextCombatTurn.o: actions/combat/GoNextCombatTurn.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/GoNextCombatTurn.cppm -o $@

$(OBJDIR)/carcer.actions.combat-RemoveCharacterFromMap.o: actions/combat/RemoveCharacterFromMap.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/RemoveCharacterFromMap.cppm -o $@

$(OBJDIR)/carcer.ui.core.o: ui/core.cppm $(OBJDIR)/carcer.ui.core-FontScale.o $(OBJDIR)/carcer.ui.core-SdlPixels.o $(OBJDIR)/carcer.ui.core-TextStyle.o $(OBJDIR)/carcer.ui.core-UiElement.o $(OBJDIR)/carcer.ui.core-colors.o $(OBJDIR)/carcer.ui.core-uiUtils.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/core.cppm -o $@

$(OBJDIR)/carcer.actions.ui.o: actions/ui/ui.cppm $(OBJDIR)/carcer.actions.ui.layers.o $(OBJDIR)/carcer.actions.ui-UiAdjustEquippedRune.o $(OBJDIR)/carcer.actions.ui-UiContinueSpecialEvent.o $(OBJDIR)/carcer.actions.ui-UiDropInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiGiveInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiPickUpItem.o $(OBJDIR)/carcer.actions.ui-UiPushFloatingNotification.o $(OBJDIR)/carcer.actions.ui-UiRemoveFloatingNotification.o $(OBJDIR)/carcer.actions.ui-UiReorderInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiSelectSpecialEventChoice.o $(OBJDIR)/carcer.actions.ui-UiSelectSpellCast.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMember.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberInventory.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberMagic.o $(OBJDIR)/carcer.actions.ui-UiSetSelectedPartyMemberId.o $(OBJDIR)/carcer.actions.ui-UiSetSpellReady.o $(OBJDIR)/carcer.actions.ui-UiToggleEquipInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiToggleManaSlotRune.o $(OBJDIR)/carcer.actions.ui-UiUpdateHeldMove.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/ui/ui.cppm -o $@

$(OBJDIR)/carcer.actions.combat-PerformCharacterDefeated.o: actions/combat/PerformCharacterDefeated.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-RemoveCharacterFromMap.o $(OBJDIR)/carcer.actions.general.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/PerformCharacterDefeated.cppm -o $@

$(OBJDIR)/carcer.layers-Layer.o: layers/Layer.cppm $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/Layer.cppm -o $@

$(OBJDIR)/carcer.ui.components-ChCompactInfo.o: ui/components/ChCompactInfo.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/ChCompactInfo.cppm -o $@

$(OBJDIR)/carcer.ui.components-MapView.o: ui/components/MapView.cppm $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/MapView.cppm -o $@

$(OBJDIR)/carcer.ui.elements-HorizontalList.o: ui/elements/HorizontalList.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/HorizontalList.cppm -o $@

$(OBJDIR)/carcer.ui.elements-OutsetRectangle.o: ui/elements/OutsetRectangle.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/OutsetRectangle.cppm -o $@

$(OBJDIR)/carcer.ui.elements-Quad.o: ui/elements/Quad.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/Quad.cppm -o $@

$(OBJDIR)/carcer.ui.elements-SpriteElement.o: ui/elements/SpriteElement.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/SpriteElement.cppm -o $@

$(OBJDIR)/carcer.ui.elements-TextLine.o: ui/elements/TextLine.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/TextLine.cppm -o $@

$(OBJDIR)/carcer.ui.elements-VerticalList.o: ui/elements/VerticalList.cppm $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/VerticalList.cppm -o $@

$(OBJDIR)/carcer.ui.helpers.o: ui/helpers/helpers.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/helpers/helpers.cppm -o $@

$(OBJDIR)/carcer.actions.combat-DoCombatActionCompletion.o: actions/combat/DoCombatActionCompletion.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-GoNextCombatTurn.o $(OBJDIR)/carcer.actions.combat-PerformCharacterDefeated.o $(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/DoCombatActionCompletion.cppm -o $@

$(OBJDIR)/carcer.layers-LayerManager.o: layers/LayerManager.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/LayerManager.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonClose.o: ui/elements/buttons/ButtonClose.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonClose.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonScroll.o: ui/elements/buttons/ButtonScroll.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonScroll.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonSprite.o: ui/elements/buttons/ButtonSprite.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-Quad.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonSprite.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonIcon.o: ui/elements/buttons/ButtonIcon.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-SpriteElement.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonIcon.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonMove.o: ui/elements/buttons/ButtonMove.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-SpriteElement.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonMove.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonWorldAction.o: ui/elements/buttons/ButtonWorldAction.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-SpriteElement.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonWorldAction.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonModal.o: ui/elements/buttons/ButtonModal.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-TextLine.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonModal.cppm -o $@

$(OBJDIR)/carcer.ui.elements-TextBanner.o: ui/elements/TextBanner.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-TextLine.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/TextBanner.cppm -o $@

$(OBJDIR)/carcer.ui.elements-TextParagraph.o: ui/elements/TextParagraph.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-Quad.o $(OBJDIR)/carcer.ui.elements-TextLine.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/TextParagraph.cppm -o $@

$(OBJDIR)/carcer.actions.combat-DoCombatAction.o: actions/combat/DoCombatAction.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-DoCombatActionCompletion.o $(OBJDIR)/carcer.actions.combat-ModifyAP.o $(OBJDIR)/carcer.actions.combat-MoveCharacter.o $(OBJDIR)/carcer.actions.combat-PerformMeleeAttack.o $(OBJDIR)/carcer.actions.combat-PerformSpellCast.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/DoCombatAction.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonList.o: ui/elements/buttons/ButtonList.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-ButtonScroll.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-TextLine.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonList.cppm -o $@

$(OBJDIR)/carcer.ui.elements-HorizontalSlider.o: ui/elements/HorizontalSlider.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-ButtonScroll.o $(OBJDIR)/carcer.ui.elements-Quad.o $(OBJDIR)/carcer.ui.elements-TextLine.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/HorizontalSlider.cppm -o $@

$(OBJDIR)/carcer.ui.elements-SectionScrollable.o: ui/elements/SectionScrollable.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-ButtonScroll.o $(OBJDIR)/carcer.ui.elements-Quad.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/SectionScrollable.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonGroup.o: ui/elements/buttons/ButtonGroup.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-ButtonModal.o $(OBJDIR)/carcer.ui.elements-ButtonSprite.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonGroup.cppm -o $@

$(OBJDIR)/carcer.ui.elements-ButtonTextWrap.o: ui/elements/buttons/ButtonTextWrap.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements-TextParagraph.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/buttons/ButtonTextWrap.cppm -o $@

$(OBJDIR)/carcer.actions.combat-DoCPUCombatTurn.o: actions/combat/DoCPUCombatTurn.cppm $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-DoCombatAction.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.game.map.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/DoCPUCombatTurn.cppm -o $@

$(OBJDIR)/carcer.ui.elements.o: ui/elements/elements.cppm $(OBJDIR)/carcer.ui.elements-ButtonClose.o $(OBJDIR)/carcer.ui.elements-ButtonGroup.o $(OBJDIR)/carcer.ui.elements-ButtonIcon.o $(OBJDIR)/carcer.ui.elements-ButtonList.o $(OBJDIR)/carcer.ui.elements-ButtonModal.o $(OBJDIR)/carcer.ui.elements-ButtonMove.o $(OBJDIR)/carcer.ui.elements-ButtonScroll.o $(OBJDIR)/carcer.ui.elements-ButtonSprite.o $(OBJDIR)/carcer.ui.elements-ButtonTextWrap.o $(OBJDIR)/carcer.ui.elements-ButtonWorldAction.o $(OBJDIR)/carcer.ui.elements-HorizontalList.o $(OBJDIR)/carcer.ui.elements-HorizontalSlider.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-Quad.o $(OBJDIR)/carcer.ui.elements-SectionScrollable.o $(OBJDIR)/carcer.ui.elements-SpriteElement.o $(OBJDIR)/carcer.ui.elements-TextBanner.o $(OBJDIR)/carcer.ui.elements-TextLine.o $(OBJDIR)/carcer.ui.elements-TextParagraph.o $(OBJDIR)/carcer.ui.elements-VerticalList.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/elements/elements.cppm -o $@

$(OBJDIR)/carcer.actions.combat.o: actions/combat/combat.cppm $(OBJDIR)/carcer.actions.combat-CharacterSetSpriteIndexOffset.o $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.combat-DoCPUCombatTurn.o $(OBJDIR)/carcer.actions.combat-DoCombatAction.o $(OBJDIR)/carcer.actions.combat-DoCombatActionCompletion.o $(OBJDIR)/carcer.actions.combat-EndCombat.o $(OBJDIR)/carcer.actions.combat-GoNextCombatTurn.o $(OBJDIR)/carcer.actions.combat-ModifyAP.o $(OBJDIR)/carcer.actions.combat-ModifyHP.o $(OBJDIR)/carcer.actions.combat-MoveCharacter.o $(OBJDIR)/carcer.actions.combat-PerformCharacterDefeated.o $(OBJDIR)/carcer.actions.combat-PerformMeleeAttack.o $(OBJDIR)/carcer.actions.combat-PerformSpellCast.o $(OBJDIR)/carcer.actions.combat-RemoveCharacterFromMap.o $(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o $(OBJDIR)/carcer.actions.combat-StartCombat.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/combat/combat.cppm -o $@

$(OBJDIR)/carcer.ui.KeyboardHeldScroll.o: ui/KeyboardHeldScroll.cppm $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/KeyboardHeldScroll.cppm -o $@

$(OBJDIR)/carcer.ui.components-BorderDropShadow.o: ui/components/borders/BorderDropShadow.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/borders/BorderDropShadow.cppm -o $@

$(OBJDIR)/carcer.ui.components-BorderInGame.o: ui/components/borders/BorderInGame.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/borders/BorderInGame.cppm -o $@

$(OBJDIR)/carcer.ui.components-InGameTitleBar.o: ui/components/InGameTitleBar.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/InGameTitleBar.cppm -o $@

$(OBJDIR)/carcer.ui.components-ItemInfo.o: ui/components/ItemInfo.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/ItemInfo.cppm -o $@

$(OBJDIR)/carcer.ui.components-TiledOverlay.o: ui/components/TiledOverlay.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/TiledOverlay.cppm -o $@

$(OBJDIR)/carcer.actions.world-PerformTownMeleeAttack.o: actions/world/PerformTownMeleeAttack.cppm $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.actions.general.o $(OBJDIR)/carcer.actions.world-ModifyPartyMemberHp.o $(OBJDIR)/carcer.actions.world-WorldSpawnDamageParticle.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/PerformTownMeleeAttack.cppm -o $@

$(OBJDIR)/carcer.ui.components-ConfirmModal.o: ui/components/ConfirmModal.cppm $(OBJDIR)/carcer.ui.components-BorderDropShadow.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/ConfirmModal.cppm -o $@

$(OBJDIR)/carcer.ui.components-FloatingNotification.o: ui/components/FloatingNotification.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.components-BorderDropShadow.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/FloatingNotification.cppm -o $@

$(OBJDIR)/carcer.ui.components-BorderInGameNarrow.o: ui/components/borders/BorderInGameNarrow.cppm $(OBJDIR)/carcer.ui.components-BorderInGame.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/borders/BorderInGameNarrow.cppm -o $@

$(OBJDIR)/carcer.ui.components-BorderInGameWide.o: ui/components/borders/BorderInGameWide.cppm $(OBJDIR)/carcer.ui.components-BorderInGame.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/borders/BorderInGameWide.cppm -o $@

$(OBJDIR)/carcer.ui.components-BorderModalSmall.o: ui/components/borders/BorderModalSmall.cppm $(OBJDIR)/carcer.ui.components-TiledOverlay.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/borders/BorderModalSmall.cppm -o $@

$(OBJDIR)/carcer.actions.world-TownEnemySeekAndMelee.o: actions/world/TownEnemySeekAndMelee.cppm $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.actions.world-PerformTownMeleeAttack.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/TownEnemySeekAndMelee.cppm -o $@

$(OBJDIR)/carcer.ui.components-BorderModalStandard.o: ui/components/borders/BorderModalStandard.cppm $(OBJDIR)/carcer.ui.components-BorderModalSmall.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/borders/BorderModalStandard.cppm -o $@

$(OBJDIR)/carcer.actions.world-TownEnemyAiAfterPlayerMove.o: actions/world/TownEnemyAiAfterPlayerMove.cppm $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.actions.world-ClearTownEnemyAiResolving.o $(OBJDIR)/carcer.actions.world-TownEnemySeekAndMelee.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/TownEnemyAiAfterPlayerMove.cppm -o $@

$(OBJDIR)/carcer.ui.components-TouchMovePad.o: ui/components/TouchMovePad.cppm $(OBJDIR)/carcer.ui.components-BorderModalStandard.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/TouchMovePad.cppm -o $@

$(OBJDIR)/carcer.actions.world-WorldMovePlayer.o: actions/world/WorldMovePlayer.cppm $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.actions.world-TownEnemyAiAfterPlayerMove.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/WorldMovePlayer.cppm -o $@

$(OBJDIR)/carcer.actions.world.o: actions/world/world.cppm $(OBJDIR)/carcer.actions.world-ClearTownEnemyAiResolving.o $(OBJDIR)/carcer.actions.world-ModifyPartyMemberHp.o $(OBJDIR)/carcer.actions.world-PerformTownMeleeAttack.o $(OBJDIR)/carcer.actions.world-TownEnemyAiAfterPlayerMove.o $(OBJDIR)/carcer.actions.world-TownEnemySeekAndMelee.o $(OBJDIR)/carcer.actions.world-WorldExamineAt.o $(OBJDIR)/carcer.actions.world-WorldInteractAt.o $(OBJDIR)/carcer.actions.world-WorldLoadActiveMap.o $(OBJDIR)/carcer.actions.world-WorldMoveActionAim.o $(OBJDIR)/carcer.actions.world-WorldMovePlayer.o $(OBJDIR)/carcer.actions.world-WorldSetActionAim.o $(OBJDIR)/carcer.actions.world-WorldSetActionMode.o $(OBJDIR)/carcer.actions.world-WorldSetCamera.o $(OBJDIR)/carcer.actions.world-WorldSetCameraMode.o $(OBJDIR)/carcer.actions.world-WorldSpawnDamageParticle.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayer.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtMarker.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtXY.o $(OBJDIR)/carcer.actions.world-WorldSpawnProjectile.o $(OBJDIR)/carcer.actions.world-WorldTalkAt.o $(OBJDIR)/carcer.actions.world-WorldTravel.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/world/world.cppm -o $@

$(OBJDIR)/carcer.actions.o: actions/actions.cppm $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.actions.general.o $(OBJDIR)/carcer.actions.ui.o $(OBJDIR)/carcer.actions.world.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c actions/actions.cppm -o $@

$(OBJDIR)/carcer.ui.ObserverRemoveLayer.o: ui/ObserverRemoveLayer.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/ObserverRemoveLayer.cppm -o $@

$(OBJDIR)/carcer.ui.ObserverSpecialEvent.o: ui/ObserverSpecialEvent.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/ObserverSpecialEvent.cppm -o $@

$(OBJDIR)/carcer.ui.components-FloatingNotificationSection.o: ui/components/FloatingNotificationSection.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.components-FloatingNotification.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/FloatingNotificationSection.cppm -o $@

$(OBJDIR)/carcer.ui.components-PartyMemberIconSelector.o: ui/components/PartyMemberIconSelector.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/PartyMemberIconSelector.cppm -o $@

$(OBJDIR)/carcer.ui.components-PartyMemberSwitcher.o: ui/components/PartyMemberSwitcher.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/PartyMemberSwitcher.cppm -o $@

$(OBJDIR)/carcer.ui.lists-ListInventory.o: ui/components/lists/ListInventory.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/lists/ListInventory.cppm -o $@

$(OBJDIR)/carcer.ui.lists-ListMagicSpells.o: ui/components/lists/ListMagicSpells.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/lists/ListMagicSpells.cppm -o $@

$(OBJDIR)/carcer.ui.lists-ListPickUp.o: ui/components/lists/ListPickUp.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/lists/ListPickUp.cppm -o $@

$(OBJDIR)/carcer.ui.components.o: ui/components/components.cppm $(OBJDIR)/carcer.ui.components-BorderDropShadow.o $(OBJDIR)/carcer.ui.components-BorderInGame.o $(OBJDIR)/carcer.ui.components-BorderInGameNarrow.o $(OBJDIR)/carcer.ui.components-BorderInGameWide.o $(OBJDIR)/carcer.ui.components-BorderModalSmall.o $(OBJDIR)/carcer.ui.components-BorderModalStandard.o $(OBJDIR)/carcer.ui.components-ChCompactInfo.o $(OBJDIR)/carcer.ui.components-ConfirmModal.o $(OBJDIR)/carcer.ui.components-FloatingNotification.o $(OBJDIR)/carcer.ui.components-FloatingNotificationSection.o $(OBJDIR)/carcer.ui.components-InGameTitleBar.o $(OBJDIR)/carcer.ui.components-ItemInfo.o $(OBJDIR)/carcer.ui.components-MapView.o $(OBJDIR)/carcer.ui.components-PartyMemberIconSelector.o $(OBJDIR)/carcer.ui.components-PartyMemberSwitcher.o $(OBJDIR)/carcer.ui.components-TiledOverlay.o $(OBJDIR)/carcer.ui.components-TouchMovePad.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/components.cppm -o $@

$(OBJDIR)/carcer.layers-LayerPopupText.o: layers/ui/LayerPopupText.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerPopupText.cppm -o $@

$(OBJDIR)/carcer.ui.layouts-ModalSmall.o: ui/layouts/ModalSmall.cppm $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/layouts/ModalSmall.cppm -o $@

$(OBJDIR)/carcer.ui.layouts-ModalStandard.o: ui/layouts/ModalStandard.cppm $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/layouts/ModalStandard.cppm -o $@

$(OBJDIR)/carcer.ui.lists-ListChCompactInfoHorizontal.o: ui/components/lists/ListChCompactInfoHorizontal.cppm $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/lists/ListChCompactInfoHorizontal.cppm -o $@

$(OBJDIR)/carcer.ui.lists-ListChCompactInfoVertical.o: ui/components/lists/ListChCompactInfoVertical.cppm $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/lists/ListChCompactInfoVertical.cppm -o $@

$(OBJDIR)/carcer.ui.popups-PopupDropConfirm.o: ui/popups/PopupDropConfirm.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/popups/PopupDropConfirm.cppm -o $@

$(OBJDIR)/carcer.ui.popups-PopupGive.o: ui/popups/PopupGive.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/popups/PopupGive.cppm -o $@

$(OBJDIR)/carcer.ui.popups-PopupInventoryItem.o: ui/popups/PopupInventoryItem.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/popups/PopupInventoryItem.cppm -o $@

$(OBJDIR)/carcer.ui.lists.o: ui/components/lists/lists.cppm $(OBJDIR)/carcer.ui.lists-ListChCompactInfoHorizontal.o $(OBJDIR)/carcer.ui.lists-ListChCompactInfoVertical.o $(OBJDIR)/carcer.ui.lists-ListInventory.o $(OBJDIR)/carcer.ui.lists-ListMagicSpells.o $(OBJDIR)/carcer.ui.lists-ListPickUp.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/components/lists/lists.cppm -o $@

$(OBJDIR)/carcer.ui.popups-PopupPickupItem.o: ui/popups/PopupPickupItem.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.popups-PopupInventoryItem.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/popups/PopupPickupItem.cppm -o $@

$(OBJDIR)/carcer.ui.popups-PopupSpellInfo.o: ui/popups/PopupSpellInfo.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.popups-PopupInventoryItem.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/popups/PopupSpellInfo.cppm -o $@

$(OBJDIR)/carcer.ui.layouts-InGameLayout.o: ui/layouts/InGameLayout.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.lists.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/layouts/InGameLayout.cppm -o $@

$(OBJDIR)/carcer.ui.popups.o: ui/popups/popups.cppm $(OBJDIR)/carcer.ui.popups-PopupDropConfirm.o $(OBJDIR)/carcer.ui.popups-PopupGive.o $(OBJDIR)/carcer.ui.popups-PopupInventoryItem.o $(OBJDIR)/carcer.ui.popups-PopupPickupItem.o $(OBJDIR)/carcer.ui.popups-PopupSpellInfo.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/popups/popups.cppm -o $@

$(OBJDIR)/carcer.ui.layouts.o: ui/layouts/layouts.cppm $(OBJDIR)/carcer.ui.layouts-InGameLayout.o $(OBJDIR)/carcer.ui.layouts-ModalSmall.o $(OBJDIR)/carcer.ui.layouts-ModalStandard.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/layouts/layouts.cppm -o $@

$(OBJDIR)/carcer.layers-LayerDropConfirm.o: layers/ui/LayerDropConfirm.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.popups.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerDropConfirm.cppm -o $@

$(OBJDIR)/carcer.layers-LayerGiveContext.o: layers/ui/LayerGiveContext.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.popups.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerGiveContext.cppm -o $@

$(OBJDIR)/carcer.layers-LayerInventoryContext.o: layers/ui/LayerInventoryContext.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.popups.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerInventoryContext.cppm -o $@

$(OBJDIR)/carcer.layers-LayerPickUpContext.o: layers/ui/LayerPickUpContext.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.popups.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerPickUpContext.cppm -o $@

$(OBJDIR)/carcer.layers-LayerSpellInfo.o: layers/ui/LayerSpellInfo.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.popups.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerSpellInfo.cppm -o $@

$(OBJDIR)/carcer.layers-LayerWorld.o: layers/ui/LayerWorld.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.layers-LayerManager.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerWorld.cppm -o $@

$(OBJDIR)/carcer.ui.minipages-MinipageCharacterSheet.o: ui/minipages/MinipageCharacterSheet.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/minipages/MinipageCharacterSheet.cppm -o $@

$(OBJDIR)/carcer.ui.minipages-MinipageEquipRunes.o: ui/minipages/MinipageEquipRunes.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/minipages/MinipageEquipRunes.cppm -o $@

$(OBJDIR)/carcer.ui.minipages-MinipageEvent.o: ui/minipages/MinipageEvent.cppm $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/minipages/MinipageEvent.cppm -o $@

$(OBJDIR)/carcer.ui.minipages-MinipagePickUp.o: ui/minipages/MinipagePickUp.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.ui.lists.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/minipages/MinipagePickUp.cppm -o $@

$(OBJDIR)/carcer.ui.minipages-MinipageSpellCast.o: ui/minipages/MinipageSpellCast.cppm $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.ui.lists.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/minipages/MinipageSpellCast.cppm -o $@

$(OBJDIR)/carcer.ui.pages-PageCharacter.o: ui/pages/PageCharacter.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/pages/PageCharacter.cppm -o $@

$(OBJDIR)/carcer.ui.pages-PageInventory.o: ui/pages/PageInventory.cppm $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.ui.lists.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/pages/PageInventory.cppm -o $@

$(OBJDIR)/carcer.ui.pages-PageMagicSetup.o: ui/pages/PageMagicSetup.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.ui.lists.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/pages/PageMagicSetup.cppm -o $@

$(OBJDIR)/carcer.ui.pages-PageTalkChoice.o: ui/pages/PageTalkChoice.cppm $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/pages/PageTalkChoice.cppm -o $@

$(OBJDIR)/carcer.ui.minipages.o: ui/minipages/minipages.cppm $(OBJDIR)/carcer.ui.minipages-MinipageCharacterSheet.o $(OBJDIR)/carcer.ui.minipages-MinipageEquipRunes.o $(OBJDIR)/carcer.ui.minipages-MinipageEvent.o $(OBJDIR)/carcer.ui.minipages-MinipagePickUp.o $(OBJDIR)/carcer.ui.minipages-MinipageSpellCast.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/minipages/minipages.cppm -o $@

$(OBJDIR)/carcer.ui.pages-PageModalEvent.o: ui/pages/PageModalEvent.cppm $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.ui.pages-PageTalkChoice.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/pages/PageModalEvent.cppm -o $@

$(OBJDIR)/carcer.layers-LayerEquipRunes.o: layers/ui/LayerEquipRunes.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.minipages.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerEquipRunes.cppm -o $@

$(OBJDIR)/carcer.layers-LayerPickUp.o: layers/ui/LayerPickUp.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.minipages.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerPickUp.cppm -o $@

$(OBJDIR)/carcer.layers-LayerSpellCast.o: layers/ui/LayerSpellCast.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.minipages.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerSpellCast.cppm -o $@

$(OBJDIR)/carcer.ui.pages.o: ui/pages/pages.cppm $(OBJDIR)/carcer.ui.pages-PageCharacter.o $(OBJDIR)/carcer.ui.pages-PageInventory.o $(OBJDIR)/carcer.ui.pages-PageMagicSetup.o $(OBJDIR)/carcer.ui.pages-PageModalEvent.o $(OBJDIR)/carcer.ui.pages-PageTalkChoice.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c ui/pages/pages.cppm -o $@

$(OBJDIR)/carcer.layers-LayerInventory.o: layers/ui/LayerInventory.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.pages.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerInventory.cppm -o $@

$(OBJDIR)/carcer.layers-LayerMagic.o: layers/ui/LayerMagic.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.ui.pages.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerMagic.cppm -o $@

$(OBJDIR)/carcer.layers-LayerSpecialEvent.o: layers/ui/LayerSpecialEvent.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.in3.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.ui.KeyboardHeldScroll.o $(OBJDIR)/carcer.ui.ObserverSpecialEvent.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.ui.pages.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/ui/LayerSpecialEvent.cppm -o $@

$(OBJDIR)/carcer.layers.o: layers/layers.cppm $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.layers-LayerDropConfirm.o $(OBJDIR)/carcer.layers-LayerEquipRunes.o $(OBJDIR)/carcer.layers-LayerGiveContext.o $(OBJDIR)/carcer.layers-LayerInventory.o $(OBJDIR)/carcer.layers-LayerInventoryContext.o $(OBJDIR)/carcer.layers-LayerMagic.o $(OBJDIR)/carcer.layers-LayerManager.o $(OBJDIR)/carcer.layers-LayerPickUp.o $(OBJDIR)/carcer.layers-LayerPickUpContext.o $(OBJDIR)/carcer.layers-LayerPopupText.o $(OBJDIR)/carcer.layers-LayerSpecialEvent.o $(OBJDIR)/carcer.layers-LayerSpellCast.o $(OBJDIR)/carcer.layers-LayerSpellInfo.o $(OBJDIR)/carcer.layers-LayerWorld.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c layers/layers.cppm -o $@

$(OBJDIR)/carcer.o: modules/carcer.cppm $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.lib.Json.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.lib.hiscore.hiscore.o $(OBJDIR)/carcer.model.instances-ItemInstance.o $(OBJDIR)/carcer.model.templates-AbilityTypes.o $(OBJDIR)/carcer.model.templates-CharacterStatDefinitions.o $(OBJDIR)/carcer.model.templates-CharacterStats.o $(OBJDIR)/carcer.model.templates-MapGrids.o $(OBJDIR)/carcer.model.templates-Maps.o $(OBJDIR)/carcer.model.templates-RuneTypes.o $(OBJDIR)/carcer.model.templates-SpecialEvents.o $(OBJDIR)/carcer.model.templates-Tileset.o $(OBJDIR)/carcer.model.templates-UtilityTypes.o $(OBJDIR)/carcer.ui.core-FontScale.o $(OBJDIR)/carcer.ui.core-SdlPixels.o $(OBJDIR)/carcer.model.templates-Abilities.o $(OBJDIR)/carcer.model.templates-StatusEffects.o $(OBJDIR)/carcer.model.templates-CharacterDerivedStats.o $(OBJDIR)/carcer.model.templates-CharacterTemplate.o $(OBJDIR)/carcer.model.templates-Items.o $(OBJDIR)/carcer.model.templates-Spells.o $(OBJDIR)/carcer.ui.core-colors.o $(OBJDIR)/carcer.model.templates-CharacterDerivedStatDefinitions.o $(OBJDIR)/carcer.ui.core-TextStyle.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.in3.o $(OBJDIR)/carcer.model.instances-TileInstance.o $(OBJDIR)/carcer.model.instances-CharacterInstance.o $(OBJDIR)/carcer.model.instances-CharacterPlayer.o $(OBJDIR)/carcer.model.instances-MapInstance.o $(OBJDIR)/carcer.model.instances-Player.o $(OBJDIR)/carcer.model.instances-Combat.o $(OBJDIR)/carcer.model.instances-World.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.general.o $(OBJDIR)/carcer.actions.ui.layers-UiCancelEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiCommitEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiRemoveLayer.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerDropContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerGiveContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventory.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventoryContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerMagic.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickUp.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickupContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPopupText.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpecialEvent.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellCast.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellInfo.o $(OBJDIR)/carcer.actions.ui-UiAdjustEquippedRune.o $(OBJDIR)/carcer.actions.ui-UiContinueSpecialEvent.o $(OBJDIR)/carcer.actions.ui-UiGiveInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiPickUpItem.o $(OBJDIR)/carcer.actions.ui-UiPushFloatingNotification.o $(OBJDIR)/carcer.actions.ui-UiRemoveFloatingNotification.o $(OBJDIR)/carcer.actions.ui-UiReorderInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiSelectSpecialEventChoice.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMember.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberInventory.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberMagic.o $(OBJDIR)/carcer.actions.ui-UiSetSelectedPartyMemberId.o $(OBJDIR)/carcer.actions.ui-UiSetSpellReady.o $(OBJDIR)/carcer.actions.ui-UiToggleEquipInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiToggleManaSlotRune.o $(OBJDIR)/carcer.actions.ui-UiUpdateHeldMove.o $(OBJDIR)/carcer.actions.world-ClearTownEnemyAiResolving.o $(OBJDIR)/carcer.actions.world-ModifyPartyMemberHp.o $(OBJDIR)/carcer.actions.world-WorldSetCamera.o $(OBJDIR)/carcer.actions.world-WorldSetCameraMode.o $(OBJDIR)/carcer.actions.world-WorldSpawnDamageParticle.o $(OBJDIR)/carcer.actions.world-WorldSpawnProjectile.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.ui.core-UiElement.o $(OBJDIR)/carcer.actions.combat-PerformMeleeAttack.o $(OBJDIR)/carcer.actions.ui.layers.o $(OBJDIR)/carcer.actions.combat-CharacterSetSpriteIndexOffset.o $(OBJDIR)/carcer.actions.combat-EndCombat.o $(OBJDIR)/carcer.actions.combat-ModifyAP.o $(OBJDIR)/carcer.actions.combat-ModifyHP.o $(OBJDIR)/carcer.actions.combat-MoveCharacter.o $(OBJDIR)/carcer.actions.combat-PerformSpellCast.o $(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o $(OBJDIR)/carcer.actions.ui-UiDropInventoryItem.o $(OBJDIR)/carcer.actions.world-WorldExamineAt.o $(OBJDIR)/carcer.actions.world-WorldInteractAt.o $(OBJDIR)/carcer.actions.world-WorldLoadActiveMap.o $(OBJDIR)/carcer.actions.world-WorldMoveActionAim.o $(OBJDIR)/carcer.actions.world-WorldSetActionAim.o $(OBJDIR)/carcer.actions.world-WorldSetActionMode.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtMarker.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtXY.o $(OBJDIR)/carcer.actions.world-WorldTalkAt.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.ui.core-uiUtils.o $(OBJDIR)/carcer.actions.ui-UiSelectSpellCast.o $(OBJDIR)/carcer.actions.combat-StartCombat.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayer.o $(OBJDIR)/carcer.actions.world-WorldTravel.o $(OBJDIR)/carcer.actions.combat-GoNextCombatTurn.o $(OBJDIR)/carcer.actions.combat-RemoveCharacterFromMap.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.actions.ui.o $(OBJDIR)/carcer.actions.combat-PerformCharacterDefeated.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.ui.components-ChCompactInfo.o $(OBJDIR)/carcer.ui.components-MapView.o $(OBJDIR)/carcer.ui.elements-HorizontalList.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-Quad.o $(OBJDIR)/carcer.ui.elements-SpriteElement.o $(OBJDIR)/carcer.ui.elements-TextLine.o $(OBJDIR)/carcer.ui.elements-VerticalList.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.actions.combat-DoCombatActionCompletion.o $(OBJDIR)/carcer.layers-LayerManager.o $(OBJDIR)/carcer.ui.elements-ButtonClose.o $(OBJDIR)/carcer.ui.elements-ButtonScroll.o $(OBJDIR)/carcer.ui.elements-ButtonSprite.o $(OBJDIR)/carcer.ui.elements-ButtonIcon.o $(OBJDIR)/carcer.ui.elements-ButtonMove.o $(OBJDIR)/carcer.ui.elements-ButtonWorldAction.o $(OBJDIR)/carcer.ui.elements-ButtonModal.o $(OBJDIR)/carcer.ui.elements-TextBanner.o $(OBJDIR)/carcer.ui.elements-TextParagraph.o $(OBJDIR)/carcer.actions.combat-DoCombatAction.o $(OBJDIR)/carcer.ui.elements-ButtonList.o $(OBJDIR)/carcer.ui.elements-HorizontalSlider.o $(OBJDIR)/carcer.ui.elements-SectionScrollable.o $(OBJDIR)/carcer.ui.elements-ButtonGroup.o $(OBJDIR)/carcer.ui.elements-ButtonTextWrap.o $(OBJDIR)/carcer.actions.combat-DoCPUCombatTurn.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.ui.KeyboardHeldScroll.o $(OBJDIR)/carcer.ui.components-BorderDropShadow.o $(OBJDIR)/carcer.ui.components-BorderInGame.o $(OBJDIR)/carcer.ui.components-InGameTitleBar.o $(OBJDIR)/carcer.ui.components-ItemInfo.o $(OBJDIR)/carcer.ui.components-TiledOverlay.o $(OBJDIR)/carcer.actions.world-PerformTownMeleeAttack.o $(OBJDIR)/carcer.ui.components-ConfirmModal.o $(OBJDIR)/carcer.ui.components-FloatingNotification.o $(OBJDIR)/carcer.ui.components-BorderInGameNarrow.o $(OBJDIR)/carcer.ui.components-BorderInGameWide.o $(OBJDIR)/carcer.ui.components-BorderModalSmall.o $(OBJDIR)/carcer.actions.world-TownEnemySeekAndMelee.o $(OBJDIR)/carcer.ui.components-BorderModalStandard.o $(OBJDIR)/carcer.actions.world-TownEnemyAiAfterPlayerMove.o $(OBJDIR)/carcer.ui.components-TouchMovePad.o $(OBJDIR)/carcer.actions.world-WorldMovePlayer.o $(OBJDIR)/carcer.actions.world.o $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.ObserverSpecialEvent.o $(OBJDIR)/carcer.ui.components-FloatingNotificationSection.o $(OBJDIR)/carcer.ui.components-PartyMemberIconSelector.o $(OBJDIR)/carcer.ui.components-PartyMemberSwitcher.o $(OBJDIR)/carcer.ui.lists-ListInventory.o $(OBJDIR)/carcer.ui.lists-ListMagicSpells.o $(OBJDIR)/carcer.ui.lists-ListPickUp.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.layers-LayerPopupText.o $(OBJDIR)/carcer.ui.layouts-ModalSmall.o $(OBJDIR)/carcer.ui.layouts-ModalStandard.o $(OBJDIR)/carcer.ui.lists-ListChCompactInfoHorizontal.o $(OBJDIR)/carcer.ui.lists-ListChCompactInfoVertical.o $(OBJDIR)/carcer.ui.popups-PopupDropConfirm.o $(OBJDIR)/carcer.ui.popups-PopupGive.o $(OBJDIR)/carcer.ui.popups-PopupInventoryItem.o $(OBJDIR)/carcer.ui.lists.o $(OBJDIR)/carcer.ui.popups-PopupPickupItem.o $(OBJDIR)/carcer.ui.popups-PopupSpellInfo.o $(OBJDIR)/carcer.ui.layouts-InGameLayout.o $(OBJDIR)/carcer.ui.popups.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.layers-LayerDropConfirm.o $(OBJDIR)/carcer.layers-LayerGiveContext.o $(OBJDIR)/carcer.layers-LayerInventoryContext.o $(OBJDIR)/carcer.layers-LayerPickUpContext.o $(OBJDIR)/carcer.layers-LayerSpellInfo.o $(OBJDIR)/carcer.layers-LayerWorld.o $(OBJDIR)/carcer.ui.minipages-MinipageCharacterSheet.o $(OBJDIR)/carcer.ui.minipages-MinipageEquipRunes.o $(OBJDIR)/carcer.ui.minipages-MinipageEvent.o $(OBJDIR)/carcer.ui.minipages-MinipagePickUp.o $(OBJDIR)/carcer.ui.minipages-MinipageSpellCast.o $(OBJDIR)/carcer.ui.pages-PageCharacter.o $(OBJDIR)/carcer.ui.pages-PageInventory.o $(OBJDIR)/carcer.ui.pages-PageMagicSetup.o $(OBJDIR)/carcer.ui.pages-PageTalkChoice.o $(OBJDIR)/carcer.ui.minipages.o $(OBJDIR)/carcer.ui.pages-PageModalEvent.o $(OBJDIR)/carcer.layers-LayerEquipRunes.o $(OBJDIR)/carcer.layers-LayerPickUp.o $(OBJDIR)/carcer.layers-LayerSpellCast.o $(OBJDIR)/carcer.ui.pages.o $(OBJDIR)/carcer.layers-LayerInventory.o $(OBJDIR)/carcer.layers-LayerMagic.o $(OBJDIR)/carcer.layers-LayerSpecialEvent.o $(OBJDIR)/carcer.layers.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c modules/carcer.cppm -o $@

clean:
	rm -rf $(OBJDIR)

CARCER_BMI_OBJ_LIST = $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.lib.Json.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.lib.hiscore.hiscore.o $(OBJDIR)/carcer.model.instances-ItemInstance.o $(OBJDIR)/carcer.model.templates-AbilityTypes.o $(OBJDIR)/carcer.model.templates-CharacterStatDefinitions.o $(OBJDIR)/carcer.model.templates-CharacterStats.o $(OBJDIR)/carcer.model.templates-MapGrids.o $(OBJDIR)/carcer.model.templates-Maps.o $(OBJDIR)/carcer.model.templates-RuneTypes.o $(OBJDIR)/carcer.model.templates-SpecialEvents.o $(OBJDIR)/carcer.model.templates-Tileset.o $(OBJDIR)/carcer.model.templates-UtilityTypes.o $(OBJDIR)/carcer.ui.core-FontScale.o $(OBJDIR)/carcer.ui.core-SdlPixels.o $(OBJDIR)/carcer.model.templates-Abilities.o $(OBJDIR)/carcer.model.templates-StatusEffects.o $(OBJDIR)/carcer.model.templates-CharacterDerivedStats.o $(OBJDIR)/carcer.model.templates-CharacterTemplate.o $(OBJDIR)/carcer.model.templates-Items.o $(OBJDIR)/carcer.model.templates-Spells.o $(OBJDIR)/carcer.ui.core-colors.o $(OBJDIR)/carcer.model.templates-CharacterDerivedStatDefinitions.o $(OBJDIR)/carcer.ui.core-TextStyle.o $(OBJDIR)/carcer.model.templates.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.in3.o $(OBJDIR)/carcer.model.instances-TileInstance.o $(OBJDIR)/carcer.model.instances-CharacterInstance.o $(OBJDIR)/carcer.model.instances-CharacterPlayer.o $(OBJDIR)/carcer.model.instances-MapInstance.o $(OBJDIR)/carcer.model.instances-Player.o $(OBJDIR)/carcer.model.instances-Combat.o $(OBJDIR)/carcer.model.instances-World.o $(OBJDIR)/carcer.model.instances.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.actions.combat-CombatAction.o $(OBJDIR)/carcer.actions.general.o $(OBJDIR)/carcer.actions.ui.layers-UiCancelEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiCommitEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiRemoveLayer.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerDropContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerEquipRunes.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerGiveContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventory.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerInventoryContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerMagic.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickUp.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPickupContext.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerPopupText.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpecialEvent.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellCast.o $(OBJDIR)/carcer.actions.ui.layers-UiShowLayerSpellInfo.o $(OBJDIR)/carcer.actions.ui-UiAdjustEquippedRune.o $(OBJDIR)/carcer.actions.ui-UiContinueSpecialEvent.o $(OBJDIR)/carcer.actions.ui-UiGiveInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiPickUpItem.o $(OBJDIR)/carcer.actions.ui-UiPushFloatingNotification.o $(OBJDIR)/carcer.actions.ui-UiRemoveFloatingNotification.o $(OBJDIR)/carcer.actions.ui-UiReorderInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiSelectSpecialEventChoice.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMember.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberInventory.o $(OBJDIR)/carcer.actions.ui-UiSetCurrentPartyMemberMagic.o $(OBJDIR)/carcer.actions.ui-UiSetSelectedPartyMemberId.o $(OBJDIR)/carcer.actions.ui-UiSetSpellReady.o $(OBJDIR)/carcer.actions.ui-UiToggleEquipInventoryItem.o $(OBJDIR)/carcer.actions.ui-UiToggleManaSlotRune.o $(OBJDIR)/carcer.actions.ui-UiUpdateHeldMove.o $(OBJDIR)/carcer.actions.world-ClearTownEnemyAiResolving.o $(OBJDIR)/carcer.actions.world-ModifyPartyMemberHp.o $(OBJDIR)/carcer.actions.world-WorldSetCamera.o $(OBJDIR)/carcer.actions.world-WorldSetCameraMode.o $(OBJDIR)/carcer.actions.world-WorldSpawnDamageParticle.o $(OBJDIR)/carcer.actions.world-WorldSpawnProjectile.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.ui.core-UiElement.o $(OBJDIR)/carcer.actions.combat-PerformMeleeAttack.o $(OBJDIR)/carcer.actions.ui.layers.o $(OBJDIR)/carcer.actions.combat-CharacterSetSpriteIndexOffset.o $(OBJDIR)/carcer.actions.combat-EndCombat.o $(OBJDIR)/carcer.actions.combat-ModifyAP.o $(OBJDIR)/carcer.actions.combat-ModifyHP.o $(OBJDIR)/carcer.actions.combat-MoveCharacter.o $(OBJDIR)/carcer.actions.combat-PerformSpellCast.o $(OBJDIR)/carcer.actions.combat-SetActiveCombatCharacter.o $(OBJDIR)/carcer.actions.ui-UiDropInventoryItem.o $(OBJDIR)/carcer.actions.world-WorldExamineAt.o $(OBJDIR)/carcer.actions.world-WorldInteractAt.o $(OBJDIR)/carcer.actions.world-WorldLoadActiveMap.o $(OBJDIR)/carcer.actions.world-WorldMoveActionAim.o $(OBJDIR)/carcer.actions.world-WorldSetActionAim.o $(OBJDIR)/carcer.actions.world-WorldSetActionMode.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtMarker.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayerAtXY.o $(OBJDIR)/carcer.actions.world-WorldTalkAt.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.ui.core-uiUtils.o $(OBJDIR)/carcer.actions.ui-UiSelectSpellCast.o $(OBJDIR)/carcer.actions.combat-StartCombat.o $(OBJDIR)/carcer.actions.world-WorldSpawnPlayer.o $(OBJDIR)/carcer.actions.world-WorldTravel.o $(OBJDIR)/carcer.actions.combat-GoNextCombatTurn.o $(OBJDIR)/carcer.actions.combat-RemoveCharacterFromMap.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.actions.ui.o $(OBJDIR)/carcer.actions.combat-PerformCharacterDefeated.o $(OBJDIR)/carcer.layers-Layer.o $(OBJDIR)/carcer.ui.components-ChCompactInfo.o $(OBJDIR)/carcer.ui.components-MapView.o $(OBJDIR)/carcer.ui.elements-HorizontalList.o $(OBJDIR)/carcer.ui.elements-OutsetRectangle.o $(OBJDIR)/carcer.ui.elements-Quad.o $(OBJDIR)/carcer.ui.elements-SpriteElement.o $(OBJDIR)/carcer.ui.elements-TextLine.o $(OBJDIR)/carcer.ui.elements-VerticalList.o $(OBJDIR)/carcer.ui.helpers.o $(OBJDIR)/carcer.actions.combat-DoCombatActionCompletion.o $(OBJDIR)/carcer.layers-LayerManager.o $(OBJDIR)/carcer.ui.elements-ButtonClose.o $(OBJDIR)/carcer.ui.elements-ButtonScroll.o $(OBJDIR)/carcer.ui.elements-ButtonSprite.o $(OBJDIR)/carcer.ui.elements-ButtonIcon.o $(OBJDIR)/carcer.ui.elements-ButtonMove.o $(OBJDIR)/carcer.ui.elements-ButtonWorldAction.o $(OBJDIR)/carcer.ui.elements-ButtonModal.o $(OBJDIR)/carcer.ui.elements-TextBanner.o $(OBJDIR)/carcer.ui.elements-TextParagraph.o $(OBJDIR)/carcer.actions.combat-DoCombatAction.o $(OBJDIR)/carcer.ui.elements-ButtonList.o $(OBJDIR)/carcer.ui.elements-HorizontalSlider.o $(OBJDIR)/carcer.ui.elements-SectionScrollable.o $(OBJDIR)/carcer.ui.elements-ButtonGroup.o $(OBJDIR)/carcer.ui.elements-ButtonTextWrap.o $(OBJDIR)/carcer.actions.combat-DoCPUCombatTurn.o $(OBJDIR)/carcer.ui.elements.o $(OBJDIR)/carcer.actions.combat.o $(OBJDIR)/carcer.ui.KeyboardHeldScroll.o $(OBJDIR)/carcer.ui.components-BorderDropShadow.o $(OBJDIR)/carcer.ui.components-BorderInGame.o $(OBJDIR)/carcer.ui.components-InGameTitleBar.o $(OBJDIR)/carcer.ui.components-ItemInfo.o $(OBJDIR)/carcer.ui.components-TiledOverlay.o $(OBJDIR)/carcer.actions.world-PerformTownMeleeAttack.o $(OBJDIR)/carcer.ui.components-ConfirmModal.o $(OBJDIR)/carcer.ui.components-FloatingNotification.o $(OBJDIR)/carcer.ui.components-BorderInGameNarrow.o $(OBJDIR)/carcer.ui.components-BorderInGameWide.o $(OBJDIR)/carcer.ui.components-BorderModalSmall.o $(OBJDIR)/carcer.actions.world-TownEnemySeekAndMelee.o $(OBJDIR)/carcer.ui.components-BorderModalStandard.o $(OBJDIR)/carcer.actions.world-TownEnemyAiAfterPlayerMove.o $(OBJDIR)/carcer.ui.components-TouchMovePad.o $(OBJDIR)/carcer.actions.world-WorldMovePlayer.o $(OBJDIR)/carcer.actions.world.o $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.ObserverRemoveLayer.o $(OBJDIR)/carcer.ui.ObserverSpecialEvent.o $(OBJDIR)/carcer.ui.components-FloatingNotificationSection.o $(OBJDIR)/carcer.ui.components-PartyMemberIconSelector.o $(OBJDIR)/carcer.ui.components-PartyMemberSwitcher.o $(OBJDIR)/carcer.ui.lists-ListInventory.o $(OBJDIR)/carcer.ui.lists-ListMagicSpells.o $(OBJDIR)/carcer.ui.lists-ListPickUp.o $(OBJDIR)/carcer.ui.components.o $(OBJDIR)/carcer.layers-LayerPopupText.o $(OBJDIR)/carcer.ui.layouts-ModalSmall.o $(OBJDIR)/carcer.ui.layouts-ModalStandard.o $(OBJDIR)/carcer.ui.lists-ListChCompactInfoHorizontal.o $(OBJDIR)/carcer.ui.lists-ListChCompactInfoVertical.o $(OBJDIR)/carcer.ui.popups-PopupDropConfirm.o $(OBJDIR)/carcer.ui.popups-PopupGive.o $(OBJDIR)/carcer.ui.popups-PopupInventoryItem.o $(OBJDIR)/carcer.ui.lists.o $(OBJDIR)/carcer.ui.popups-PopupPickupItem.o $(OBJDIR)/carcer.ui.popups-PopupSpellInfo.o $(OBJDIR)/carcer.ui.layouts-InGameLayout.o $(OBJDIR)/carcer.ui.popups.o $(OBJDIR)/carcer.ui.layouts.o $(OBJDIR)/carcer.layers-LayerDropConfirm.o $(OBJDIR)/carcer.layers-LayerGiveContext.o $(OBJDIR)/carcer.layers-LayerInventoryContext.o $(OBJDIR)/carcer.layers-LayerPickUpContext.o $(OBJDIR)/carcer.layers-LayerSpellInfo.o $(OBJDIR)/carcer.layers-LayerWorld.o $(OBJDIR)/carcer.ui.minipages-MinipageCharacterSheet.o $(OBJDIR)/carcer.ui.minipages-MinipageEquipRunes.o $(OBJDIR)/carcer.ui.minipages-MinipageEvent.o $(OBJDIR)/carcer.ui.minipages-MinipagePickUp.o $(OBJDIR)/carcer.ui.minipages-MinipageSpellCast.o $(OBJDIR)/carcer.ui.pages-PageCharacter.o $(OBJDIR)/carcer.ui.pages-PageInventory.o $(OBJDIR)/carcer.ui.pages-PageMagicSetup.o $(OBJDIR)/carcer.ui.pages-PageTalkChoice.o $(OBJDIR)/carcer.ui.minipages.o $(OBJDIR)/carcer.ui.pages-PageModalEvent.o $(OBJDIR)/carcer.layers-LayerEquipRunes.o $(OBJDIR)/carcer.layers-LayerPickUp.o $(OBJDIR)/carcer.layers-LayerSpellCast.o $(OBJDIR)/carcer.ui.pages.o $(OBJDIR)/carcer.layers-LayerInventory.o $(OBJDIR)/carcer.layers-LayerMagic.o $(OBJDIR)/carcer.layers-LayerSpecialEvent.o $(OBJDIR)/carcer.layers.o $(OBJDIR)/carcer.o

