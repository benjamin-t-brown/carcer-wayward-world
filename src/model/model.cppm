module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

export module carcer.model;

export import bmin.containers;
import carcer.data;
import carcer.db;
import carcer.game.map.TileFields;
import sdl2w;

export {

// --- ItemInstance.cppm ---

// --- from model/instances/ItemInstance.h ---
namespace model {

struct ItemInstance {
  bmin::String id;
  bmin::String itemTemplateName;
  int quantity = 1;
  int x = 0;
  int y = 0;
};

} // namespace model

// --- CharacterInstance.cppm ---

// --- from model/instances/CharacterInstance.h ---
namespace model {

enum class CharacterFacing { Right, Left };

// Up, up-right, right, down-right → Right; left, up-left, down-left, down → Left.
CharacterFacing facingFromMoveDelta(int dx, int dy);

// Map-entity character (player avatar, future NPCs). Distinct from CharacterPlayer
// (party chrome). Position is tile coords in map space, same as TileInstance.
struct CharacterInstance {
  bmin::String id;
  bmin::String name;
  bmin::String templateName;
  int x = 0;
  int y = 0;
  // Original map spawn tile; used to persist defeated map-placed characters after
  // movement.
  int spawnX = -1;
  int spawnY = -1;
  // Combat runtime (meaningful while world.combat.active).
  int currentAp = 0;
  int currentHp = 0; // enemies only; party HP lives on CharacterPlayer
  // Cached from CharacterTemplate.combat.hp (enemies/NPCs); party HP lives on
  // CharacterPlayer.
  int maxHp = 0;
  // True once currentHp has been set from combat (distinguishes 0 HP from uninitialized).
  bool hpInitialized = false;
  // enemies/NPCs only; party MP lives on CharacterPlayer.
  int currentMp = 0;
  // Cached from CharacterTemplate.combat.mp (enemies/NPCs).
  int maxMp = 0;
  // True once currentMp has been set from combat (distinguishes 0 MP from uninitialized).
  bool mpInitialized = false;
  // Transient pose offset (e.g. weapon swing frame); reset via
  // CharacterSetSpriteIndexOffset.
  int spriteIndexOffset = 0;
  // Default art faces right; left uses horizontal flip at render time.
  CharacterFacing facing = CharacterFacing::Right;
  // Map AI: set when IMMOBILE_UNTIL_ENEMY_SPOTTED spots the party (not persisted).
  bool agitated = false;

  // Cached from CharacterTemplate at spawn / active-map hoist (for AI without DB
  // lookups).
  CharacterTemplateType type = CharacterTemplateType::TOWNSPERSON;
  bmin::String label;
  bmin::String behaviorName;
  int visionRadius = 0;
  CombatBehaviorName combatBehaviorTown = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combatBehaviorCombat = CombatBehaviorName::SEEK_AND_MELEE;
  CharacterStats stats;
};

bool characterInstanceIsEnemy(const CharacterInstance& character);
void updateCharacterFacingFromMove(CharacterInstance& character, int dx, int dy);
void updateCharacterFacingToward(CharacterInstance& character, int targetX, int targetY);
bool isCharacterFacingLeft(const CharacterInstance& character);

/** Copy AI/faction fields from template onto a map character instance. */
void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate);

/** Lookup templateName on the database and apply; returns false if missing. */
bool tryApplyCharacterTemplateToInstance(CharacterInstance& character,
                                         const db::Database& database);

} // namespace model

// --- CharacterPlayer.cppm ---

// --- from model/instances/CharacterPlayer.h ---
namespace model {

struct CharacterPlayerEquipment {
  // these represent ids of items inside the character's inventory
  bmin::String weapon0Id;
  bmin::String weapon1Id;
  bmin::String ammoId;
  bmin::String hatId;
  bmin::String garbId;
  bmin::String glovesId;
  bmin::String pantsId;
  bmin::String shoesId;
  bmin::String necklaceId;
  bmin::String shieldId;
};

struct CharacterInventoryItem {
  bmin::String itemName;
  bmin::String id;
  int quantity;
};

/** Owned rune tally on the character (not bag inventory). */
struct CharacterAvailableRune {
  RuneType type = RuneType::HEAT;
  int count = 0;
};

struct CharacterPlayer;
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

struct CharacterPlayer {
  static constexpr size_t kRuneSlotCount = 8;

  bmin::String instanceId;
  bmin::String name;
  bmin::String templateName;
  bmin::DynArray<CharacterInventoryItem> inventory;
  CharacterPlayerEquipment equipment;
  CharacterTemplate params;
  CharacterStats stats;
  int currentHp = 0;
  int currentMp = 0;
  bmin::DynArray<bmin::String> knownSpells;
  bmin::DynArray<bmin::String> readySpells;
  // Tallies of runes the character owns; equip capacity comes from here.
  bmin::DynArray<CharacterAvailableRune> availableRunes;
  // Dense list of equipped rune types; size 0..kRuneSlotCount, no mid-list holes.
  bmin::DynArray<RuneType> equippedRunes;

  CharacterPlayer(const CharacterTemplate& params = CharacterTemplate(),
                  const bmin::DynArray<CharacterInventoryItem>& inventory = {},
                  const CharacterPlayerEquipment& equipment = {});
};

bmin::String characterPlayerGetSprite(const CharacterPlayer& characterPlayer);
bmin::String characterPlayerGetSpriteAtIndexOffset(const CharacterPlayer& characterPlayer,
                                                   int indexOffset);

enum class EquipItemResult {
  EQUIPPED,
  UNEQUIPPED,
  NOT_EQUIPPABLE,
  ITEM_NOT_IN_INVENTORY,
  SLOT_OCCUPIED,
  TWO_HANDED_OFF_HAND,
};

enum class CharacterEquipmentSlot {
  WEAPON0,
  WEAPON1,
  AMMO,
  HAT,
  GARB,
  GLOVES,
  PANTS,
  SHOES,
  NECKLACE,
  SHIELD,
};

std::optional<CharacterEquipmentSlot>
characterPlayerGetEquipmentSlotForItemId(const CharacterPlayer& characterPlayer,
                                         const bmin::String& itemId);
bmin::String characterEquipmentSlotAbbrev(CharacterEquipmentSlot slot);
bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const bmin::String& itemId);
EquipItemResult characterPlayerToggleEquipItem(CharacterPlayer& characterPlayer,
                                               const bmin::String& itemId,
                                               const db::Database& database);

enum class EquipRuneResult {
  EQUIPPED,
  UNEQUIPPED,
  NOT_A_RUNE,
  ITEM_NOT_IN_INVENTORY,
  SLOT_OCCUPIED,
  SLOT_EMPTY,
  INVALID_SLOT,
  ALREADY_EQUIPPED,
  NO_RUNE_AVAILABLE,
};

int characterPlayerCountAvailableRunesOfType(const CharacterPlayer& characterPlayer,
                                            RuneType runeType);
void characterPlayerSetAvailableRuneCount(CharacterPlayer& characterPlayer,
                                          RuneType runeType,
                                          int count);
int characterPlayerCountEquippedRunesOfType(const CharacterPlayer& characterPlayer,
                                            RuneType runeType);
bool characterPlayerCanEquipRuneType(const CharacterPlayer& characterPlayer,
                                     RuneType runeType);
std::optional<RuneType>
characterPlayerFindFirstEquippableRuneType(const CharacterPlayer& characterPlayer);
/** Insert `runeType` (sorted by type) when list has room and capacity remains. */
EquipRuneResult characterPlayerEquipRuneType(CharacterPlayer& characterPlayer,
                                             RuneType runeType);
/** Erase at `slotIndex` and compact; fails if index is empty / out of range. */
EquipRuneResult characterPlayerUnequipRuneFromSlot(CharacterPlayer& characterPlayer,
                                                   size_t slotIndex);
/** Unequip the last equipped occurrence of `runeType`, if any. */
EquipRuneResult characterPlayerUnequipOneRuneOfType(CharacterPlayer& characterPlayer,
                                                     RuneType runeType);
/**
 * Filled slot (`slotIndex < size`): unequip and compact.
 * First empty (`slotIndex == size`): append first equippable available rune type.
 * `slotIndex > size` or past max: INVALID_SLOT.
 */
EquipRuneResult characterPlayerToggleManaSlotRune(CharacterPlayer& characterPlayer,
                                                   size_t slotIndex);

std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const bmin::String& itemName);
void characterPlayerAddItemToInventory(CharacterPlayer& characterPlayer,
                                       const model::ItemTemplate& itemTemplate,
                                       int quantity = 1);
void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const bmin::String& itemName,
                                                  int quantity = 1);
void characterPlayerRemoveItemFromInventoryById(CharacterPlayer& characterPlayer,
                                                const bmin::String& itemId,
                                                int quantity = 1);

enum class GiveItemResult {
  SUCCESS,
  ITEM_NOT_FOUND,
  INVALID_QUANTITY,
  TOO_HEAVY,
};

GiveItemResult characterPlayerGiveInventoryItem(CharacterPlayer& from,
                                                CharacterPlayer& to,
                                                const bmin::String& itemId,
                                                int quantity,
                                                const db::Database& database);
bool characterPlayerReorderInventoryItem(CharacterPlayer& characterPlayer,
                                         size_t index,
                                         int direction);
int characterGetWeightCarrying(const CharacterPlayer& characterPlayer,
                               const db::Database* database);
int characterGetWeightCapacity(const CharacterPlayer& characterPlayer);
int characterGetRationSlotCapacity(const CharacterPlayer& characterPlayer,
                                   const db::Database& database);

/** Copy starting known/ready spell lists from template onto a party member. */
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

} // namespace model

// --- Player.cppm ---

// --- from model/instances/Player.h ---
namespace model {
struct Player {
  bmin::String name;
  bmin::DynArray<model::CharacterPlayer> party;
  int currentPartyMemberIndex = 0;
  int currentPartyMemberInventoryIndex = 0;
  int currentPartyMemberMagicIndex = 0;
  int gold = 0;
  int food = 0;
};
CharacterPlayer* playerFindPartyMemberById(Player& _player, const bmin::String& _id);
CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index);
/** Index of party member with instanceId, or -1 if not found. */
int playerFindPartyMemberIndexById(const Player& _player, const bmin::String& _id);

} // namespace model

// --- TileInstance.cppm ---

// --- from model/instances/TileInstance.h ---
namespace model {

struct TileInstance {
  bmin::String id;
  bmin::String tilesetName;
  int tileId = 0;
  int x = 0;
  int y = 0;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  bool isExplored = false;
  bool isVisible = false;
  bool isContainer = false;
  bool isWalkable = false;
  bmin::DynArray<game::TileField> fields;
};

} // namespace model

// --- MapInstance.cppm ---

// --- from model/instances/MapInstance.h ---
namespace model {

enum class TurnMode { TURN_TOWN, TURN_OUTDOOR, TURN_COMBAT };

// Session-scoped fog-of-war memory for a map template (one bit per cell).
struct ExploredMapMask {
  int width = 0;
  int height = 0;
  bmin::DynArray<uint8_t> bits;
};

// Open-door tileId mutation on a map (closed doors become tileId+1 at runtime).
struct OpenedDoorRecord {
  int layer = 0;
  int x = 0;
  int y = 0;
  int tileId = 0;
};

// Map-placed character removed for the session (matched on template + spawn tile).
struct DefeatedCharacterRecord {
  bmin::String templateName;
  int x = 0;
  int y = 0;
};

// Tile overlay fields persisted per layer/cell.
struct PersistentTileFieldRecord {
  int layer = 0;
  int x = 0;
  int y = 0;
  bmin::DynArray<game::TileField> fields;
};

struct PersistentMapState {
  int version = 2;
  ExploredMapMask explored;
  bmin::DynArray<OpenedDoorRecord> openedDoors;
  bmin::DynArray<DefeatedCharacterRecord> defeatedCharacters;
  bmin::DynArray<PersistentTileFieldRecord> tileFields;

  bmin::Map<int, bmin::DynArray<TileInstance>> tiles;
  bmin::DynArray<CharacterInstance> characters;
  bmin::DynArray<ItemInstance> items;
};

struct MapInstance {
  bmin::String id;
  bmin::String label;
  bmin::String templateName;

  PersistentMapState persistentState;
  int width = 0;
  int height = 0;
  int spriteWidth = 0;
  int spriteHeight = 0;
  int tileLayerNumber = 0;
  MapType mapType = MapType::TOWN;
};

struct TileXY {
  int x = 0;
  int y = 0;
};

using TileLayerMap = bmin::Map<int, bmin::DynArray<TileInstance>>;

TileLayerMap& mapInstanceTiles(MapInstance& map);
const TileLayerMap& mapInstanceTiles(const MapInstance& map);
bool mapHasLayer(const TileLayerMap& layers, int layer);
bool mapInstanceHasLayer(const TileLayerMap& layers, int layer);
bmin::DynArray<TileInstance>& mapLayerAt(TileLayerMap& layers, int layer);
const bmin::DynArray<TileInstance>* mapLayerPtr(const TileLayerMap& layers, int layer);
bmin::DynArray<TileInstance>* mapLayerPtr(TileLayerMap& layers, int layer);
TileInstance* mapInstanceGetTileAt(MapInstance& map, int x, int y, int layer);
const TileInstance* mapInstanceGetTileAt(const MapInstance& map, int x, int y, int layer);
TileXY mapInstanceGetMinMaxLayer(const MapInstance& map);

MapInstance createMapInstanceFromTemplate(const CarcerMapTemplate& mapTemplate);

// Flat cell index → tile (x, y); matches createMapInstanceFromTemplate math.
TileXY tileIndexToXY(int i, int width);
int tileXYToIndex(int x, int y, int width);

// First marker whose name matches (ceditor findMarkerOnMap semantics).
const MapMarkerPlacement* findMarkerOnTemplate(const CarcerMapTemplate& mapTemplate,
                                               const bmin::String& markerName);

CharacterInstance* mapInstanceFindCharacter(MapInstance& map, const bmin::String& id);
const CharacterInstance* mapInstanceFindCharacter(const MapInstance& map,
                                                  const bmin::String& id);

} // namespace model

namespace game {

void ageMapInstanceTileFields(model::MapInstance& map, int steps);
void agePersistentTileFieldRecords(bmin::DynArray<model::PersistentTileFieldRecord>& records,
                                   int steps);
void addTileField(model::TileInstance& tile, TileFieldType type);
void addTileFieldAt(model::MapInstance& map, int tileX, int tileY, TileFieldType type);

} // namespace game

// --- Combat.cppm ---

// --- from model/Combat.h ---
namespace model {


inline constexpr int COMBAT_STARTING_AP = 4;
inline constexpr int COMBAT_MOVE_COST = 1;
inline constexpr int COMBAT_ATTACK_COST = 4;
inline constexpr int COMBAT_MELEE_DAMAGE = 10;
inline constexpr int COMBAT_HIT_CHANCE_PERCENT = 75;

enum class CombatActionType { MOVE, SHOOT, SPELL, WAIT };

struct SpellTargetInfo {
  bmin::String targetCharacterId;
  int tileX = 0;
  int tileY = 0;
};

struct Combat {
  bool active = false;
  bmin::DynArray<bmin::String> turnOrderIds;
  int activeTurnIndex = 0;
  bmin::String activeCharacterId;
  bool isWaitingForAction = false;
};

void removeCharacterFromCombatTurnOrder(Combat& combat, const bmin::String& characterId);

bool isPartyMember(const Player& player, const bmin::String& characterId);
bool isCharacterAlly(const Player& player, const CharacterInstance& character);
bool isCharacterEnemy(const CharacterInstance& character);

int getCharacterHp(const Player& player, const CharacterInstance& character);
void setCharacterHp(Player& player, CharacterInstance& character, int hp);
/** Updates party member HP by instance id without requiring them on the map. */
bool modifyPartyMemberHp(Player& player, const bmin::String& instanceId, int delta);
bool isCharacterDefeated(const Player& player, const CharacterInstance& character);

} // namespace model

// --- World.cppm ---

// --- from model/instances/World.h ---
namespace model {

enum class CameraMode { Follow, Aiming, Dragging, Controlled };

enum class WorldActionMode { NONE, EXAMINE, TALK, SPELL };

  // Map-space hit feedback: splash animation plus a numeric label (not UI floating text).
struct DamageParticle {
  bmin::String animationName;
  bmin::String text;
  int tileX = 0;
  int tileY = 0;
  TimerStruct lifetime;
};

// Traveling combat projectile (tile-space lerp from caster to zone origin).
struct WorldProjectile {
  bmin::String animationName;
  bmin::String text;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  ProjectilePath projectilePath = ProjectilePath::PROJECTILE_PATH_NONE;
  TimerStruct travel;
  int yOffset = 0;
};

struct CameraInfo {
  int camX = 0; // map pixel space
  int camY = 0;
  CameraMode cameraMode = CameraMode::Follow;
  // empty = auto-resolve to current party member avatar when cameraMode is Follow
  bmin::String cameraFollowCharacterId;
  int viewW = 0; // MapView content size in map-pixel space (unscaled)
  int viewH = 0;
};

struct ActiveMap {
  bmin::String gridId;
  int mapLayer = 0;
  bmin::DynArray<CharacterInstance> characters;
  bmin::DynArray<ItemInstance> items;
  // bmin::DynArray<TileField> fields;
  bmin::DynArray<DamageParticle> damageParticles;
  bmin::DynArray<WorldProjectile> projectiles;
};

struct World {
  ActiveMap activeMap;

  CameraInfo camera;
  WorldActionMode actionMode = WorldActionMode::NONE;
  // Meaningful only when actionMode != NONE (Examine / Talk / Spell aim cursor).
  std::optional<TileXY> actionAimTile;
  // Meaningful only when actionMode == SPELL (spell template name, e.g. "FLAME").
  bmin::String pendingSpellId;
  bmin::String pendingChId;

  // True while town enemy AI is resolving (seek / melee swing / particles).
  // Blocks player movement and world actions until the timed sequence finishes.
  bool resolvingTownEnemyAi = false;

  Combat combat;
};

void resetAllCombatAp(World& world, int ap = COMBAT_STARTING_AP);
void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database);
void removeExtraPartyMembersFromMap(World& world, const Player& player);

Combat createCombatFromWorld(const World& world, const Player& player);

bmin::String formatCharacterLogLabel(const ActiveMap& activeMap, const bmin::String& id);

} // namespace model

} // export
