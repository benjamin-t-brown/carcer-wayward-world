module;
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

export module carcer.data;

export import bmin.containers;

export {

// --- templates/AbilityTypes.cppm ---

// --- from model/templates/AbilityTypes.h ---
namespace model {

enum class StatusEffectCondition {
  CONDITION_ALWAYS,
  CONDITION_TARGET_HP_BELOW_HALF,
  CONDITION_SELF_HP_BELOW_HALF,
  CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET,
  CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND,
  CONDITION_ATTACK_MISSED,
  CONDITION_FIRST_TIME_ATTACKED,
};

enum class AbilityType {
  ABILITY_ATTACK,
  ABILITY_SPELL,
  ABILITY_SKILL,
  ABILITY_SUB_ATTACK,
};

enum class TargetSelectType {
  TARGET_SELF,
  TARGET_UNIT,
  TARGET_ZONE,
  TARGET_ALL_IN_RANGE,
};

enum class TargetAllegianceSelectType {
  TARGET_ALLEGIANCE_SAME,
  TARGET_ALLEGIANCE_SAME_AND_SELF,
  TARGET_ALLEGIANCE_OTHER,
  TARGET_ALLEGIANCE_ALL,
  TARGET_ALLEGIANCE_ALL_AND_SELF,
};

enum class Dice { D0, D2, D4, D6, D8, D10, D12, D20, D100 };

enum class StatsEnum { STAT_STR, STAT_MND, STAT_CON, STAT_AGI, STAT_LCK };

enum class StatusEventType {
  STATUS_EVENT_ON_APPLIED,
  STATUS_EVENT_ON_ATTACK,
  STATUS_EVENT_ON_ATTACKED_MELEE,
  STATUS_EVENT_ON_ATTACKED_RANGE,
  STATUS_EVENT_ON_ATTACKED_MAGIC,
  STATUS_EVENT_ON_MOVE,
  STATUS_EVENT_ON_TURN_START,
  STATUS_EVENT_ON_TURN_END,
  STATUS_EVENT_ON_ROUND_START,
};

enum class CurrentStatEnum {
  CURRENT_STAT_HP,
  CURRENT_STAT_AP,
  CURRENT_STAT_MANA,
  CURRENT_STAT_AC,
};

enum class StatusActionTargetType {
  STATUS_ACTION_TARGET_SELF,
  STATUS_ACTION_TARGET_ATTACKER_LOCATION,
  STATUS_ACTION_TARGET_LAST_LOCATION,
};

enum class AbilityCostType {
  ABILITY_COST_NONE,
  ABILITY_COST_MANA,
  ABILITY_COST_COOLDOWN,
  ABILITY_COST_HP,
};

enum class AttackClass {
  ATTACK_CLASS_MELEE,
  ATTACK_CLASS_RANGED,
  ATTACK_CLASS_MAGIC,
  ATTACK_CLASS_AUTO_HIT,
};

enum class DamageType {
  DAMAGE_TYPE_EDGED,
  DAMAGE_TYPE_BASHING,
  DAMAGE_TYPE_PIERCING,
  DAMAGE_TYPE_HEAT,
  DAMAGE_TYPE_FREEZE,
  DAMAGE_TYPE_STATIC,
  DAMAGE_TYPE_NECROTIC,
  DAMAGE_TYPE_EPHEMERAL,
  DAMAGE_TYPE_TRUE,
};

enum class ProjectilePath {
  PROJECTILE_PATH_SHORT,
  PROJECTILE_PATH_MEDIUM,
  PROJECTILE_PATH_TALL,
  PROJECTILE_PATH_NONE,
};

enum class ProjectileType {
  PROJECTILE_NONE,
  DOT_RED,
  DOT_WHITE,
  DOT_BLUE,
  DOT_BIG_YELLOW,
  COMET_BLUE,
  COMET_RED,
  COMET_GREEN,
  ARROW_FIRE,
  ARROW_NORMAL,
};

struct TargetSelectInfoPoint {
  int x = 0;
  int y = 0;
};

struct TargetSelectInfo {
  TargetSelectType targetType = TargetSelectType::TARGET_SELF;
  TargetAllegianceSelectType allegianceSelectType =
      TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME;
  int numTargetableUnits = 1;
  TargetSelectInfoPoint zoneSize;
  int range = 0;
};

struct Stats {
  int STR = 0;
  int MND = 0;
  int CON = 0;
  int AGI = 0;
  int LCK = 0;
};

struct CurrentStats {
  int HP = 0;
  int AP = 0;
  int MANA = 0;
  int AC = 0;
};

struct Resistance {
  DamageType attackType = DamageType::DAMAGE_TYPE_EDGED;
  int mod = 0;
};

struct AbilitySave {
  StatsEnum saveStat = StatsEnum::STAT_STR;
  int saveBase = 0;
  StatsEnum saveAgainst = StatsEnum::STAT_STR;
  int saveAgainstBase = 0;
};

struct AbilityAttackDmg {
  bmin::DynArray<Dice> dmgDice;
  int dmgBonus = 0;
  StatsEnum dmgStat = StatsEnum::STAT_STR;
  float dmgStatMult = 0.f;
  int attackBonus = 0;
};

struct AbilityAttack {
  AttackClass attackClass = AttackClass::ATTACK_CLASS_MELEE;
  DamageType damageType = DamageType::DAMAGE_TYPE_EDGED;
  std::optional<AbilityAttackDmg> dmg;
  std::optional<AbilitySave> save;
};

struct AbilityStatus {
  bmin::String statusEffect;
  std::optional<AbilitySave> save;
  std::optional<int> baseDuration;
  std::optional<int> durationBonus;
};

struct AbilityRestore {
  CurrentStatEnum restoreWhich = CurrentStatEnum::CURRENT_STAT_HP;
  bmin::DynArray<Dice> restoreDice;
  int restoreBonus = 0;
  StatsEnum restoreStat = StatsEnum::STAT_STR;
  int restoreStatMult = 0;
};

struct AbilityDamage {
  DamageType damageType = DamageType::DAMAGE_TYPE_EDGED;
  bmin::DynArray<Dice> dmgDice;
  int dmgBonus = 0;
  StatsEnum dmgStat = StatsEnum::STAT_STR;
  float dmgStatMult = 0.f;
};

struct AbilityDepiction {
  bmin::String dmgAnim;
  ProjectileType projectileType = ProjectileType::PROJECTILE_NONE;
  ProjectilePath projectilePath = ProjectilePath::PROJECTILE_PATH_NONE;
  bmin::String startSound;
  bmin::String dmgSound;
};

StatusEffectCondition statusEffectConditionFromString(const bmin::String& value);
bmin::String statusEffectConditionToString(StatusEffectCondition value);

AbilityType abilityTypeFromString(const bmin::String& value);
bmin::String abilityTypeToString(AbilityType value);

TargetSelectType targetSelectTypeFromString(const bmin::String& value);
bmin::String targetSelectTypeToString(TargetSelectType value);

TargetAllegianceSelectType targetAllegianceSelectTypeFromString(const bmin::String& value);
bmin::String targetAllegianceSelectTypeToString(TargetAllegianceSelectType value);

Dice diceFromString(const bmin::String& value);
bmin::String diceToString(Dice value);

StatsEnum statsEnumFromString(const bmin::String& value);
bmin::String statsEnumToString(StatsEnum value);

StatusEventType statusEventTypeFromString(const bmin::String& value);
bmin::String statusEventTypeToString(StatusEventType value);

CurrentStatEnum currentStatEnumFromString(const bmin::String& value);
bmin::String currentStatEnumToString(CurrentStatEnum value);

StatusActionTargetType statusActionTargetTypeFromString(const bmin::String& value);
bmin::String statusActionTargetTypeToString(StatusActionTargetType value);

AbilityCostType abilityCostTypeFromString(const bmin::String& value);
bmin::String abilityCostTypeToString(AbilityCostType value);

AttackClass attackClassFromString(const bmin::String& value);
bmin::String attackClassToString(AttackClass value);

DamageType damageTypeFromString(const bmin::String& value);
bmin::String damageTypeToString(DamageType value);

ProjectilePath projectilePathFromString(const bmin::String& value);
bmin::String projectilePathToString(ProjectilePath value);

bool projectileTypeHasFacing(ProjectileType value);
bmin::String projectileTypeToAnimBase(ProjectileType value);
ProjectileType projectileTypeFromString(const bmin::String& value);
ProjectileType projectileTypeFromAnimName(const bmin::String& animName);
bmin::String projectileTypeToString(ProjectileType value);

} // namespace model

// --- templates/Abilities.cppm ---

// --- from model/templates/Abilities.h ---
namespace model {

struct AbilityTemplate {
  bmin::String name;
  bmin::String label;
  bmin::String description;
  bmin::String icon;
  AbilityType type = AbilityType::ABILITY_ATTACK;
  TargetSelectInfo targetSelect;
  int apCost = 0;
  AbilityCostType costType = AbilityCostType::ABILITY_COST_NONE;
  int costValue = 0;
  AbilityDepiction depiction;
  bmin::DynArray<AbilityAttack> attacks;
  bmin::DynArray<AbilityStatus> statuses;
  bmin::DynArray<AbilityRestore> restores;
  bmin::DynArray<AbilityDamage> damages;
};

} // namespace model

// --- stats/CharacterStats.cppm ---

// --- from model/stats/CharacterStats.h ---
namespace model {

struct GenericCombatStats {
  int str = 0;
  int mnd = 0;
  int con = 0;
  int agi = 0;
  int lck = 0;
};

struct WeaponMasteryStats {
  int edged = 0;
  int pole = 0;
  int blunt = 0;
  int range = 0;
  int unarmed = 0;
};

struct MagicMasteryStats {
  int mana = 0;
  int abilityPower = 0;
  int attunement = 0;
  int faith = 0;
  int lore = 0;
};

struct BodyMasteryStats {
  int resistPhysical = 0;
  int resistMagical = 0;
  int healingEffectiveness = 0;
  int dr = 0;
  int armorTraining = 0;
};

struct TrainableCombatStats {
  WeaponMasteryStats weapon;
  MagicMasteryStats magic;
  BodyMasteryStats body;
};

struct CharacterSkills {
  int trickery = 0;
  int stealth = 0;
  int social = 0;
  int magicItemUse = 0;
  int cooking = 0;
  int acrobatics = 0;
  int survival = 0;
  int focus = 0;
  int conditioning = 0;
};

struct CharacterStats {
  GenericCombatStats generic;
  TrainableCombatStats trainable;
  CharacterSkills skills;
};

} // namespace model

// --- stats/CharacterDerivedStats.cppm ---

// --- from model/stats/CharacterDerivedStats.h ---
namespace model {

struct CharacterDerivedStats {
  int hp = 10;
  int maxMana = 10;
  int actionPoints = 1;
  int mightDamage = 1;
  int magicDamage = 0;
  int attackHitChancePercent = 50;
  int abilityPower = 0;
  int damageReduction = 0;
  int armorClass = 10;
  int spellPotency = 1;
  bmin::String resistancesSummary = "None";
  int jumpDistance = 2;
  int healingEffectivenessPercent = 100;
  int statusEffectShield = 0;
  int materiaSlots = 0;
  int shieldBonus = 0;
  int enemyVisionRange = 0;
  int mageLore = 0;
  int toolUse = 0;
  int tradeDiscountPercent = 0;
  int itemUsagePercent = 100;
  int foodConsumption = 100;
  int firstAidPerLevel = 1;
  int ingredientFindChancePercent = 10;
  int foodConsumptionPerDay = 100;
};

CharacterDerivedStats computeCharacterDerivedStats(const CharacterStats& stats,
                                                   int characterLevel = 1);

} // namespace model

// --- stats/CharacterDerivedStatDefinitions.cppm ---

// --- from model/stats/CharacterDerivedStatDefinitions.h ---
namespace model {

struct CharacterDerivedStatDefinitions {
  static bmin::String derivedTitle();
  static bmin::String derivedSkillsTitle();

  static bmin::String actionPointsLabel();
  static bmin::String actionPointsDescription();
  static bmin::String actionPointsValue(const CharacterDerivedStats& derived);

  static bmin::String mightDamageLabel();
  static bmin::String mightDamageDescription();
  static bmin::String mightDamageValue(const CharacterDerivedStats& derived);

  static bmin::String magicDamageLabel();
  static bmin::String magicDamageDescription();
  static bmin::String magicDamageValue(const CharacterDerivedStats& derived);

  static bmin::String hpLabel();
  static bmin::String hpDescription();
  static bmin::String hpValue(const CharacterDerivedStats& derived);

  static bmin::String attackHitChanceLabel();
  static bmin::String attackHitChanceDescription();
  static bmin::String attackHitChanceValue(const CharacterDerivedStats& derived);

  static bmin::String abilityPowerLabel();
  static bmin::String abilityPowerDescription();
  static bmin::String abilityPowerValue(const CharacterDerivedStats& derived);

  static bmin::String damageReductionLabel();
  static bmin::String damageReductionDescription();
  static bmin::String damageReductionValue(const CharacterDerivedStats& derived);

  static bmin::String armorClassLabel();
  static bmin::String armorClassDescription();
  static bmin::String armorClassValue(const CharacterDerivedStats& derived);

  static bmin::String spellPotencyLabel();
  static bmin::String spellPotencyDescription();
  static bmin::String spellPotencyValue(const CharacterDerivedStats& derived);

  static bmin::String manaLabel();
  static bmin::String manaDescription();
  static bmin::String manaValue(const CharacterDerivedStats& derived);

  static bmin::String resistancesLabel();
  static bmin::String resistancesDescription();
  static bmin::String resistancesValue(const CharacterDerivedStats& derived);

  static bmin::String jumpDistanceLabel();
  static bmin::String jumpDistanceDescription();
  static bmin::String jumpDistanceValue(const CharacterDerivedStats& derived);

  static bmin::String healingEffectivenessLabel();
  static bmin::String healingEffectivenessDescription();
  static bmin::String healingEffectivenessValue(const CharacterDerivedStats& derived);

  static bmin::String statusEffectShieldLabel();
  static bmin::String statusEffectShieldDescription();
  static bmin::String statusEffectShieldValue(const CharacterDerivedStats& derived);

  static bmin::String materiaSlotsLabel();
  static bmin::String materiaSlotsDescription();
  static bmin::String materiaSlotsValue(const CharacterDerivedStats& derived);

  static bmin::String shieldBonusLabel();
  static bmin::String shieldBonusDescription();
  static bmin::String shieldBonusValue(const CharacterDerivedStats& derived);

  static bmin::String enemyVisionRangeLabel();
  static bmin::String enemyVisionRangeDescription();
  static bmin::String enemyVisionRangeValue(const CharacterDerivedStats& derived);

  static bmin::String mageLoreLabel();
  static bmin::String mageLoreDescription();
  static bmin::String mageLoreValue(const CharacterDerivedStats& derived);

  static bmin::String toolUseLabel();
  static bmin::String toolUseDescription();
  static bmin::String toolUseValue(const CharacterDerivedStats& derived);

  static bmin::String tradeDiscountLabel();
  static bmin::String tradeDiscountDescription();
  static bmin::String tradeDiscountValue(const CharacterDerivedStats& derived);

  static bmin::String itemUsageLabel();
  static bmin::String itemUsageDescription();
  static bmin::String itemUsageValue(const CharacterDerivedStats& derived);

  static bmin::String foodConsumptionLabel();
  static bmin::String foodConsumptionDescription();
  static bmin::String foodConsumptionValue(const CharacterDerivedStats& derived);

  static bmin::String firstAidLabel();
  static bmin::String firstAidDescription();
  static bmin::String firstAidValue(const CharacterDerivedStats& derived);

  static bmin::String ingredientFindChanceLabel();
  static bmin::String ingredientFindChanceDescription();
  static bmin::String ingredientFindChanceValue(const CharacterDerivedStats& derived);

  static bmin::String foodConsumptionPerDayLabel();
  static bmin::String foodConsumptionPerDayDescription();
  static bmin::String foodConsumptionPerDayValue(const CharacterDerivedStats& derived);
};

} // namespace model

// --- stats/CharacterStatDefinitions.cppm ---

// --- from model/stats/CharacterStatDefinitions.h ---
namespace model {

struct CharacterStatDefinitions {
  static bmin::String attributesTitle();
  static bmin::String strengthLabel();
  static bmin::String strengthDescription();
  static bmin::String agilityLabel();
  static bmin::String agilityDescription();
  static bmin::String constitutionLabel();
  static bmin::String constitutionDescription();
  static bmin::String mindLabel();
  static bmin::String mindDescription();
  static bmin::String luckLabel();
  static bmin::String luckDescription();

  static bmin::String weaponMasteryTitle();
  static bmin::String edgedWeaponsLabel();
  static bmin::String edgedWeaponsDescription();
  static bmin::String poleWeaponsLabel();
  static bmin::String poleWeaponsDescription();
  static bmin::String bluntWeaponsLabel();
  static bmin::String bluntWeaponsDescription();
  static bmin::String rangeWeaponsLabel();
  static bmin::String rangeWeaponsDescription();
  static bmin::String unarmedLabel();
  static bmin::String unarmedDescription();

  static bmin::String magicMasteryTitle();
  static bmin::String manaLabel();
  static bmin::String manaDescription();
  static bmin::String abilityPowerLabel();
  static bmin::String abilityPowerDescription();
  static bmin::String attunementLabel();
  static bmin::String attunementDescription();
  static bmin::String faithLabel();
  static bmin::String faithDescription();
  static bmin::String loreLabel();
  static bmin::String loreDescription();

  static bmin::String bodyMasteryTitle();
  static bmin::String resistPhysicalLabel();
  static bmin::String resistPhysicalDescription();
  static bmin::String resistMagicalLabel();
  static bmin::String resistMagicalDescription();
  static bmin::String healingEffLabel();
  static bmin::String healingEffDescription();
  static bmin::String damageReductionLabel();
  static bmin::String damageReductionDescription();
  static bmin::String armorTrainingLabel();
  static bmin::String armorTrainingDescription();

  static bmin::String skillsTitle();
  static bmin::String trickeryLabel();
  static bmin::String trickeryDescription();
  static bmin::String stealthLabel();
  static bmin::String stealthDescription();
  static bmin::String socialLabel();
  static bmin::String socialDescription();
  static bmin::String magicItemUseLabel();
  static bmin::String magicItemUseDescription();
  static bmin::String cookingLabel();
  static bmin::String cookingDescription();
  static bmin::String acrobaticsLabel();
  static bmin::String acrobaticsDescription();
  static bmin::String survivalLabel();
  static bmin::String survivalDescription();
  static bmin::String focusLabel();
  static bmin::String focusDescription();
  static bmin::String conditioningLabel();
  static bmin::String conditioningDescription();
};

} // namespace model

// --- templates/RuneTypes.cppm ---

// --- from model/templates/RuneTypes.h ---
namespace model {

enum class RuneType {
  HEAT = 0,
  ENTROPY,
  REGROWTH,
  DISPLACE,
  EXPAND,
  ATTACH,
  TRANSFORM,
  COMPACT,
};

inline constexpr int kRuneTypeCount = 8;

RuneType runeTypeFromString(const bmin::String& value);
bmin::String runeTypeToString(RuneType value);

int runeTypeIndex(RuneType value);
RuneType runeTypeFromIndex(int index);

bmin::String runeTypeToSpriteName(RuneType value);

} // namespace model

// --- templates/CharacterTemplate.cppm ---

// --- from model/templates/CharacterTemplate.h ---
namespace model {

enum class CharacterTemplateType {
  TOWNSPERSON,
  TOWNSPERSON_STATIC,
  ENEMY,
  ENEMY_STATIC,
};

enum class CharacterTemplateBehaviorName {
  MOVE_RANDOMLY,
  IMMOBILE,
  IMMOBILE_UNTIL_ENEMY_SPOTTED,
  SEEK_MARKER,
  MOVE_LEFT_RIGHT,
  MOVE_UP_DOWN,
};

enum class CombatBehaviorName {
  SEEK_AND_MELEE,
};

struct CharacterTemplateTalk {
  bmin::String talkName;
  bmin::String portraitName;
};

struct CharacterTemplateBehavior {
  bmin::String behaviorName;
};

struct CharacterTemplateCombat {
  int hp = 0;
  int mp = 0;
  bmin::String dropTable;
};

struct CharacterTemplateCombatBehavior {
  CombatBehaviorName town = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combat = CombatBehaviorName::SEEK_AND_MELEE;
};

struct CharacterTemplateSound {
  bmin::String deathSoundName;
  bmin::String weaponSoundName;
};

struct CharacterTemplateStatus {
  bmin::String status;
};

struct CharacterTemplateVision {
  int radius = 0;
};

struct CharacterTemplate {
  CharacterTemplateType type;
  bmin::String name;
  bmin::String label;
  bmin::String spritesheetName;
  bmin::String spriteOffset;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterStats stats;
  CharacterTemplateCombat combat;
  CharacterTemplateCombatBehavior combatBehavior;
  CharacterTemplateSound sound;
  bmin::DynArray<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
  /** SpellTemplate.name values applied when constructing a CharacterPlayer. */
  bmin::DynArray<bmin::String> startingKnownSpells;
  /** Optional prepared subset; only names also in known after apply are kept. */
  bmin::DynArray<bmin::String> startingReadySpells;
};

bmin::String characterGetSprite(const CharacterTemplate& character);
bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                             int indexOffset);

/** Initialize CharacterStats from a template's stats block. */
void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl);

} // namespace model

// --- templates/Items.cppm ---

// --- from model/templates/Items.h ---
namespace model {

enum class ItemType {
  WEAPON_MELEE,
  WEAPON_MELEE_2H,
  WEAPON_RANGED,
  WEAPON_AMMO,
  SHIELD,
  GARB,
  PANTS,
  GLOVES,
  HAT,
  SHOES,
  NECKLACE,
  POTION,
  UTILITY,
  RUNE,
  UNKNOWN
};

bmin::String getStringFromItemType(ItemType itemType);
ItemType getItemTypeFromString(const bmin::String& itemTypeString);

bool itemTypeIsEquippable(ItemType itemType);
bool itemTypeIsTwoHandedWeapon(ItemType itemType);
bool itemTypeUsesWeaponSlots(ItemType itemType);
bool itemTypeUsesRuneSlots(ItemType itemType);

enum class ItemUsability {
  NOT_USABLE,
  USABLE_EVERYWHERE,
  USABLE_TOWN_ONLY,
  USABLE_COMBAT_ONLY,
  USABLE_OUTSIDE_ONLY,
  USABLE_TOWN_AND_COMBAT,
};

ItemUsability getItemUsabilityFromString(const bmin::String& value);

struct ItemWeaponConfig {
  bmin::String abilityName;
  bmin::DynArray<AbilityAttackDmg> dmgOverrides;
};

struct ItemUseAbilityConfig {
  bmin::String abilityName;
  bmin::DynArray<AbilityAttackDmg> dmgOverrides;
  bmin::DynArray<AbilityRestore> restoreOverrides;
};

struct ItemTemplate {
  ItemType itemType = ItemType::UNKNOWN;
  bmin::String name;
  bmin::String label;
  bmin::String iconSpriteName;
  bmin::String description;
  int weight = 0;
  int value = 0;
  bool stackable = false;
  bool indestructable = false;
  ItemUsability itemUsability = ItemUsability::NOT_USABLE;
  std::optional<ItemUseAbilityConfig> useAbility;
  std::optional<bmin::String> useSpecialEvent;
  bmin::DynArray<bmin::String> statusEffectNames;
  std::optional<ItemWeaponConfig> weapon;
  std::optional<RuneType> runeType;
};

} // namespace model

// --- templates/MapGrids.cppm ---

// --- from model/templates/MapGrids.h ---
namespace model {

// Matches ceditor MapGridTemplate / assets/db/map-grids.json.
// cells is row-major [y][x]; empty string = unassigned slot.
struct MapGridTemplate {
  bmin::String name;
  bmin::String label;
  int gridWidth = 1;
  int gridHeight = 1;
  int mapWidth = 1;
  int mapHeight = 1;
  bmin::DynArray<bmin::DynArray<bmin::String>> cells;
};

} // namespace model

// --- templates/Maps.cppm ---

// --- from model/templates/Maps.h ---
namespace model {

struct CarcerMapTileTemplate;

struct MapTileRef {
  int l = 0;
  int i = 0;
};

struct MapTileItemEntry {
  bmin::String name;
  int quantity = 1;
};

// Overlay icon drawn on the tile in MapView when the tile is visible.
// Missing / unknown JSON → HIDDEN.
enum class TileOverlayVisibility {
  HIDDEN,
  SHOW_EVENT_ON_TILE,
  SHOW_TRAVEL_UP,
  SHOW_TRAVEL_DOWN,
};

bmin::String getStringFromTileOverlayVisibility(TileOverlayVisibility visibility);
TileOverlayVisibility getTileOverlayVisibilityFromString(const bmin::String& value);

// Sprite name for MapView overlay, or empty when HIDDEN.
bmin::String tileOverlayVisibilitySpriteName(TileOverlayVisibility visibility);

struct TileEventTrigger {
  bmin::String eventId;
  bool requiresNonCombat = true;
  bool requiresLook = false;
  TileOverlayVisibility overlayVisibility = TileOverlayVisibility::HIDDEN;
};

struct TravelTrigger {
  bmin::String destinationMapName;
  bmin::String destinationMarkerName;
  int destinationX = 0;
  int destinationY = 0;
  int destinationLayer = 0;
  bool requiresAction = false;
  TileOverlayVisibility overlayVisibility = TileOverlayVisibility::HIDDEN;
};

struct TileLightSource {
  float angle = 0.0f;
  float intensity = 0.0f;
  int radius = 0;
};

struct TileOverrides {
  std::optional<bool> isWalkableOverride;
  std::optional<bool> isSeeThroughOverride;
  std::optional<bool> isContainerOverride;
  std::optional<TileLightSource> lightSourceOverride;
};

enum class MapType {
  TOWN,
  OUTDOOR,
};

bmin::String getStringFromMapType(MapType mapType);
MapType getMapTypeFromString(const bmin::String& mapTypeString);

struct MapCharacterPlacement : MapTileRef {
  bmin::String name;
};

struct MapItemPlacement : MapTileRef {
  bmin::String name;
  int quantity = 1;
};

struct MapMarkerPlacement : MapTileRef {
  bmin::String name;
};

struct MapEventTriggerPlacement : MapTileRef, TileEventTrigger {};

struct MapTravelTriggerPlacement : MapTileRef, TravelTrigger {};

struct MapTileOverridePlacement : MapTileRef {
  TileOverrides overrides;
};

struct MapLightSourcePlacement : MapTileRef, TileLightSource {};

struct CarcerMapTemplate {
  CarcerMapTemplate() {}
  bmin::String name;
  bmin::String label;
  MapType type = MapType::TOWN;
  int width = 0;
  int height = 0;
  int spriteWidth = 28;
  int spriteHeight = 32;
  bmin::DynArray<bmin::String> tilesets;
  bmin::DynArray<int> layers;
  bmin::DynArray<bmin::DynArray<int>> tiles;
  bmin::DynArray<MapCharacterPlacement> characters;
  bmin::DynArray<MapItemPlacement> items;
  bmin::DynArray<MapMarkerPlacement> markers;
  bmin::DynArray<MapEventTriggerPlacement> eventTriggers;
  bmin::DynArray<MapTravelTriggerPlacement> travelTriggers;
  bmin::DynArray<MapTileOverridePlacement> tileOverrides;
  bmin::DynArray<MapLightSourcePlacement> lightSources;
};

struct CarcerMapTileTemplate {
  bmin::DynArray<bmin::String> characters;
  bmin::DynArray<MapTileItemEntry> items;
  bmin::DynArray<bmin::String> markers;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  bmin::String tilesetName;
  int tileId = 0;
};

} // namespace model

// --- templates/SpecialEvents.cppm ---

// --- from model/templates/SpecialEvents.h ---
namespace model {

enum class GameEventType { MODAL, TALK };
enum class GameEventChildType { KEYWORD, CHOICE, END, EXEC, SWITCH };
// enum class KeywordType { K, K_DUP, K_SWITCH, K_CHILD };

struct VariableValue;

// struct GameEventChildKeyword;
struct GameEventChildChoice;
struct GameEventChildSwitch;
struct GameEventChildExec;
struct GameEventChildEnd;
struct Variable;

// Discriminated union for GameEventChild
using GameEventChild = std::variant<GameEventChildChoice,
                                    GameEventChildSwitch,
                                    GameEventChildExec,
                                    GameEventChildEnd>;

struct GameEvent {
  bmin::String id;
  bmin::String title;
  GameEventType eventType; // indicates which ui layer to use for the event
  bmin::String icon;        // name of sprite to use for the event
  // a mapping from variable name to its original text and it's evaluated value
  bmin::DynArray<Variable> vars;
  bmin::DynArray<GameEventChild> children;
};

struct Variable {
  bmin::String id;
  bmin::String key;
  bmin::String value;
  bmin::String importFrom;
};

struct VariableValue {
  bmin::String str;
  std::optional<bmin::String> evaluated;
};

struct AudioInfo {
  bmin::String audioName;
  int volume;
  int offset;
};

// struct KeywordDataK {
//   KeywordType keywordType = KeywordType::K;
//   bmin::String text;
// };

// struct KeywordDataKDup {
//   KeywordType keywordType = KeywordType::K_DUP;
//   bmin::String keyword;
// };

// struct KeywordCheck {
//   bmin::String conditionStr;
//   bmin::String next;
// };

// struct KeywordDataKSwitch {
//   KeywordType keywordType = KeywordType::K_SWITCH;
//   bmin::String defaultNext; // id of next child
//   bmin::DynArray<KeywordCheck> checks;
// };

// struct KeywordDataKChild {
//   KeywordType keywordType = KeywordType::K_CHILD;
//   bmin::String next; // id of next child
// };

// // Discriminated union for KeywordData
// struct KeywordData {
//   KeywordType keywordType;
//   // Union of possible data - only one should be populated based on keywordType
//   std::optional<KeywordDataK> k;
//   std::optional<KeywordDataKDup> kDup;
//   std::optional<KeywordDataKSwitch> kSwitch;
//   std::optional<KeywordDataKChild> kChild;
// };

// struct GameEventChildKeyword {
//   GameEventChildType eventChildType = GameEventChildType::KEYWORD;
//   bmin::String id;
//   std::map<bmin::String, KeywordData> keywords;
// };

struct ChoiceSwitchText {
  bmin::String conditionStr;
  bmin::String text;
};

struct Choice {
  bmin::String text;
  bmin::DynArray<ChoiceSwitchText> switchText;
  bmin::String prefixText;
  bmin::String conditionStr;
  bmin::String evalStr;
  bmin::String next;
};

struct GameEventChildChoice {
  GameEventChildType eventChildType = GameEventChildType::CHOICE;
  bmin::String id;
  bmin::String text;
  bmin::DynArray<Choice> choices;
  std::optional<AudioInfo> audioInfo;
};

struct SwitchCase {
  bmin::String conditionStr;
  bmin::String next;
};

struct GameEventChildSwitch {
  GameEventChildType eventChildType = GameEventChildType::SWITCH;
  bmin::String id;
  bmin::String defaultNext;
  bmin::DynArray<SwitchCase> cases;
};

struct GameEventChildExec {
  GameEventChildType eventChildType = GameEventChildType::EXEC;
  bmin::String id;
  bmin::DynArray<bmin::String> paragraphs;
  bmin::String execStr;
  bmin::String next;
  bool autoAdvance;
  std::optional<AudioInfo> audioInfo;
};

struct GameEventChildEnd {
  GameEventChildType eventChildType = GameEventChildType::END;
  bmin::String id;
  bmin::String next;
};

} // namespace model

// --- templates/Spells.cppm ---

// --- from model/templates/Spells.h ---
namespace model {

struct SpellRuneRequirement {
  RuneType type = RuneType::HEAT;
  int count = 1;
};

struct SpellTemplate {
  bmin::String name;
  bmin::String label;
  bmin::String description;
  bmin::String icon;
  bmin::String abilityName;
  bmin::DynArray<SpellRuneRequirement> requiredRunes;
};

} // namespace model

// --- templates/StatusEffects.cppm ---

// --- from model/templates/StatusEffects.h ---
namespace model {

struct StatusEffectEvent {
  StatusEventType type = StatusEventType::STATUS_EVENT_ON_APPLIED;
  StatusEffectCondition condition = StatusEffectCondition::CONDITION_ALWAYS;
};

struct StatusEffectDurationScale {
  StatsEnum durationStat = StatsEnum::STAT_MND;
  int durationStatMult = 0;
};

struct StatusEffectAction {
  StatusActionTargetType statusActionTargetType =
      StatusActionTargetType::STATUS_ACTION_TARGET_SELF;
  bmin::String abilityName;
  bmin::DynArray<StatusEffectEvent> events;
};

struct StatusEffectTemplate {
  bmin::String name;
  bmin::String description;
  // Default turns before apply-time modifiers (applier spellPotency, victim shield, feats).
  int baseDuration = 0;
  std::optional<StatusEffectDurationScale> durationScale;
  std::optional<Stats> applyBonuses;
  std::optional<CurrentStats> applyCurrentStatChange;
  bmin::DynArray<Resistance> applyResistances;
  bmin::DynArray<StatusEffectAction> actions;
};

} // namespace model

// --- templates/Tileset.cppm ---

// --- from model/templates/Tileset.h ---
namespace model {

struct TileMetadata;

enum TileStepSound {
  TILE_STEP_SOUND_FLOOR,
  TILE_STEP_SOUND_GRASS,
  TILE_STEP_SOUND_DIRT,
  TILE_STEP_SOUND_GRAVEL,
};

struct TileMetadata {
  int id;
  bmin::String description = "";
  TileStepSound stepSound = TILE_STEP_SOUND_FLOOR;
  bool isWalkable = true;
  bool isSeeThrough = true;
  bool isDoor = false;
  bool isContainer = false;
};

struct TilesetTemplate {
  bmin::String name;
  bmin::String spriteBase;
  int tileWidth;
  int tileHeight;
  bmin::DynArray<TileMetadata> tiles;
};

} // namespace model

// --- templates/UtilityTypes.cppm ---

// --- from model/templates/UtilityTypes.h ---
namespace model {

struct TimerStruct {
  int duration = 1000;
  int t = 0;

  TimerStruct(int duration = 1000);
};

bmin::String createRandomId();
void timerStructStart(TimerStruct& timer, int duration = 0);
void timerStructRestart(TimerStruct& timer);
void timerStructUpdate(TimerStruct& timer, int deltaTimeMs);
bool timerStructIsComplete(const TimerStruct& timer);
double timerStructGetPct(const TimerStruct& timer);

} // namespace model

// Persisted application score data is part of the data boundary; it does not
// need a standalone module with no consumers.
namespace hiscore {

struct HiscoreRow {
  bmin::String name;
  int score;
};

bmin::DynArray<HiscoreRow> getHighScores();
void saveHighScores(const bmin::DynArray<HiscoreRow>& hiscores);

} // namespace hiscore

} // export
