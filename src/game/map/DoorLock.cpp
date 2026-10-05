#include "game/map/DoorLock.h"

#include "game/inventory/SpecialEventItemsStorage.h"
#include "in3/EventRunnerHelpers.h"
#include "bmin/StringInterop.h"
#include <algorithm>

namespace game {
namespace {

bool isLockToolStack(const model::CharacterInventoryItem& stack,
                     const db::Database& database) {
  if (stack.quantity <= 0) {
    return false;
  }
  const auto* itemTemplate = database.findItemTemplate(bmin::toStringView(stack.itemName));
  return itemTemplate != nullptr && itemTemplate->isLockTool;
}

int countLockToolQuantity(const model::CharacterPlayer& leader,
                          const db::Database& database) {
  auto total = int{0};
  for (const auto& stack : leader.inventory) {
    if (!isLockToolStack(stack, database)) {
      continue;
    }
    total += stack.quantity;
  }
  return total;
}

bool playerHasKeyItem(const model::CharacterPlayer& leader,
                      const bmin::Map<bmin::String, bmin::String>& specialEventStorage,
                      const bmin::String& keyItem,
                      const db::Database& database) {
  if (keyItem.empty()) {
    return false;
  }
  if (database.findItemTemplate(bmin::toStringView(keyItem)) == nullptr) {
    return false;
  }
  for (const auto& stack : leader.inventory) {
    if (stack.itemName == keyItem && stack.quantity > 0) {
      return true;
    }
  }
  const auto storageKey = bmin::String(kVarsItemsPrefix) + keyItem;
  if (const auto value = in3::getStorage(specialEventStorage, storageKey)) {
    if (specialItemQuantityIsPresent(*value)) {
      return true;
    }
  }
  return false;
}

void consumeLockTools(model::CharacterPlayer& leader,
                      int quantity,
                      const db::Database& database) {
  auto remaining = quantity;
  auto index = size_t{0};
  while (index < leader.inventory.size() && remaining > 0) {
    auto& stack = leader.inventory[index];
    if (!isLockToolStack(stack, database)) {
      index += 1;
      continue;
    }
    if (stack.quantity > remaining) {
      stack.quantity -= remaining;
      remaining = 0;
    } else {
      remaining -= stack.quantity;
      leader.inventory.erase(index);
    }
  }
}

int toolsRequiredForLock(int lockLevel, int trickery) {
  return std::max(0, lockLevel - trickery);
}

int bashMaxForLeader(int brutishness) {
  return (brutishness / 5) * 5;
}

} // namespace

ClosedDoorBumpInfo classifyClosedDoorBump(
    const model::TileInstance& door,
    const model::CharacterPlayer& leader,
    const bmin::Map<bmin::String, bmin::String>& specialEventStorage,
    const db::Database& database) {
  auto info = ClosedDoorBumpInfo{};
  if (!door.doorLock.has_value()) {
    info.outcome = ClosedDoorBumpOutcome::OpenImmediateSilent;
    return info;
  }

  const auto& lock = *door.doorLock;
  if (lock.lockLevel == 0) {
    if (!playerHasKeyItem(leader, specialEventStorage, lock.keyItem, database)) {
      info.outcome = ClosedDoorBumpOutcome::InfoMissingKey;
      return info;
    }
    info.outcome = ClosedDoorBumpOutcome::ConfirmKeyUnlock;
    return info;
  }

  info.toolsRequired = toolsRequiredForLock(lock.lockLevel, leader.stats.skills.trickery);
  if (info.toolsRequired <= 0) {
    info.outcome = ClosedDoorBumpOutcome::OpenImmediateLockpick;
    return info;
  }

  if (lock.lockLevel <= bashMaxForLeader(leader.stats.skills.brutishness)) {
    info.outcome = ClosedDoorBumpOutcome::OpenImmediateBash;
    return info;
  }

  if (countLockToolQuantity(leader, database) < info.toolsRequired) {
    info.outcome = ClosedDoorBumpOutcome::InfoInsufficientTools;
    return info;
  }

  info.outcome = ClosedDoorBumpOutcome::ConfirmToolUnlock;
  return info;
}

ClosedDoorOpenResult tryOpenClosedDoor(
    model::TileInstance& door,
    model::CharacterPlayer& leader,
    const bmin::Map<bmin::String, bmin::String>& specialEventStorage,
    const db::Database& database) {
  if (!door.doorLock.has_value()) {
    door.tileId = door.tileId + 1;
    return ClosedDoorOpenResult::OpenedSilent;
  }

  const auto& lock = *door.doorLock;
  if (lock.lockLevel == 0) {
    if (!playerHasKeyItem(leader, specialEventStorage, lock.keyItem, database)) {
      return ClosedDoorOpenResult::Blocked;
    }
    door.doorLock = std::nullopt;
    return ClosedDoorOpenResult::OpenedKey;
  }

  const auto toolsRequired =
      toolsRequiredForLock(lock.lockLevel, leader.stats.skills.trickery);
  if (toolsRequired <= 0) {
    door.doorLock = std::nullopt;
    return ClosedDoorOpenResult::OpenedLockpick;
  }

  if (lock.lockLevel <= bashMaxForLeader(leader.stats.skills.brutishness)) {
    door.doorLock = std::nullopt;
    door.tileId = door.tileId + 1;
    return ClosedDoorOpenResult::OpenedBash;
  }

  if (countLockToolQuantity(leader, database) < toolsRequired) {
    return ClosedDoorOpenResult::Blocked;
  }

  consumeLockTools(leader, toolsRequired, database);
  door.doorLock = std::nullopt;
  return ClosedDoorOpenResult::OpenedLockpick;
}

} // namespace game
