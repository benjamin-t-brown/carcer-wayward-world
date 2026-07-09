#include "LoadAbilityJson.h"
#include <stdexcept>

namespace db {

bmin::DynArray<model::Dice> parseDiceArray(const Json& json, const String& fieldName) {
  if (!json.contains(fieldName.cStr()) || !json[fieldName.cStr()].is_array()) {
    throw std::runtime_error((String("Missing or invalid field: ") + fieldName).cStr());
  }
  bmin::DynArray<model::Dice> dice;
  for (const auto& dieJson : json[fieldName.cStr()]) {
    if (!dieJson.is_string()) {
      throw std::runtime_error((String("Dice entry must be a string in ") + fieldName).cStr());
    }
    dice.pushBack(model::diceFromString(dieJson.get<String>()));
  }
  return dice;
}

model::TargetSelectInfo parseTargetSelectInfo(const Json& json) {
  model::TargetSelectInfo info;
  if (!json.contains("targetType") || !json["targetType"].is_string()) {
    throw std::runtime_error("targetSelect missing targetType");
  }
  info.targetType = model::targetSelectTypeFromString(json["targetType"].get<String>());
  if (!json.contains("allegianceSelectType") || !json["allegianceSelectType"].is_string()) {
    throw std::runtime_error("targetSelect missing allegianceSelectType");
  }
  info.allegianceSelectType =
      model::targetAllegianceSelectTypeFromString(json["allegianceSelectType"].get<String>());
  if (!json.contains("numTargetableUnits") || !json["numTargetableUnits"].is_number_integer()) {
    throw std::runtime_error("targetSelect missing numTargetableUnits");
  }
  info.numTargetableUnits = json["numTargetableUnits"].get<int>();
  if (!json.contains("zoneSize") || !json["zoneSize"].is_object()) {
    throw std::runtime_error("targetSelect missing zoneSize");
  }
  const auto& zoneSize = json["zoneSize"];
  if (!zoneSize.contains("x") || !zoneSize["x"].is_number_integer()) {
    throw std::runtime_error("zoneSize missing x");
  }
  if (!zoneSize.contains("y") || !zoneSize["y"].is_number_integer()) {
    throw std::runtime_error("zoneSize missing y");
  }
  info.zoneSize.x = zoneSize["x"].get<int>();
  info.zoneSize.y = zoneSize["y"].get<int>();
  if (!json.contains("range") || !json["range"].is_number_integer()) {
    throw std::runtime_error("targetSelect missing range");
  }
  info.range = json["range"].get<int>();
  return info;
}

model::Stats parseStats(const Json& json) {
  model::Stats stats;
  if (json.contains("STR") && json["STR"].is_number_integer()) {
    stats.STR = json["STR"].get<int>();
  }
  if (json.contains("MND") && json["MND"].is_number_integer()) {
    stats.MND = json["MND"].get<int>();
  }
  if (json.contains("CON") && json["CON"].is_number_integer()) {
    stats.CON = json["CON"].get<int>();
  }
  if (json.contains("AGI") && json["AGI"].is_number_integer()) {
    stats.AGI = json["AGI"].get<int>();
  }
  if (json.contains("LCK") && json["LCK"].is_number_integer()) {
    stats.LCK = json["LCK"].get<int>();
  }
  return stats;
}

model::CurrentStats parseCurrentStats(const Json& json) {
  model::CurrentStats stats;
  if (json.contains("HP") && json["HP"].is_number_integer()) {
    stats.HP = json["HP"].get<int>();
  }
  if (json.contains("AP") && json["AP"].is_number_integer()) {
    stats.AP = json["AP"].get<int>();
  }
  if (json.contains("MANA") && json["MANA"].is_number_integer()) {
    stats.MANA = json["MANA"].get<int>();
  }
  if (json.contains("AC") && json["AC"].is_number_integer()) {
    stats.AC = json["AC"].get<int>();
  }
  return stats;
}

model::Resistance parseResistance(const Json& json) {
  model::Resistance resistance;
  if (!json.contains("attackType") || !json["attackType"].is_string()) {
    throw std::runtime_error("Resistance missing attackType");
  }
  resistance.attackType = model::damageTypeFromString(json["attackType"].get<String>());
  if (!json.contains("mod") || !json["mod"].is_number_integer()) {
    throw std::runtime_error("Resistance missing mod");
  }
  resistance.mod = json["mod"].get<int>();
  return resistance;
}

model::AbilitySave parseAbilitySave(const Json& json) {
  model::AbilitySave save;
  if (!json.contains("saveStat") || !json["saveStat"].is_string()) {
    throw std::runtime_error("AbilitySave missing saveStat");
  }
  save.saveStat = model::statsEnumFromString(json["saveStat"].get<String>());
  if (!json.contains("saveBase") || !json["saveBase"].is_number_integer()) {
    throw std::runtime_error("AbilitySave missing saveBase");
  }
  save.saveBase = json["saveBase"].get<int>();
  if (!json.contains("saveAgainst") || !json["saveAgainst"].is_string()) {
    throw std::runtime_error("AbilitySave missing saveAgainst");
  }
  save.saveAgainst = model::statsEnumFromString(json["saveAgainst"].get<String>());
  if (!json.contains("saveAgainstBase") || !json["saveAgainstBase"].is_number_integer()) {
    throw std::runtime_error("AbilitySave missing saveAgainstBase");
  }
  save.saveAgainstBase = json["saveAgainstBase"].get<int>();
  return save;
}

model::AbilityAttackDmg parseAbilityAttackDmg(const Json& json) {
  model::AbilityAttackDmg dmg;
  dmg.dmgDice = parseDiceArray(json, "dmgDice");
  if (!json.contains("dmgBonus") || !json["dmgBonus"].is_number_integer()) {
    throw std::runtime_error("AbilityAttackDmg missing dmgBonus");
  }
  dmg.dmgBonus = json["dmgBonus"].get<int>();
  if (!json.contains("dmgStat") || !json["dmgStat"].is_string()) {
    throw std::runtime_error("AbilityAttackDmg missing dmgStat");
  }
  dmg.dmgStat = model::statsEnumFromString(json["dmgStat"].get<String>());
  if (!json.contains("dmgStatMult") || !json["dmgStatMult"].is_number_integer()) {
    throw std::runtime_error("AbilityAttackDmg missing dmgStatMult");
  }
  dmg.dmgStatMult = json["dmgStatMult"].get<int>();
  if (!json.contains("attackBonus") || !json["attackBonus"].is_number_integer()) {
    throw std::runtime_error("AbilityAttackDmg missing attackBonus");
  }
  dmg.attackBonus = json["attackBonus"].get<int>();
  return dmg;
}

model::AbilityAttack parseAbilityAttack(const Json& json) {
  model::AbilityAttack attack;
  if (!json.contains("attackClass") || !json["attackClass"].is_string()) {
    throw std::runtime_error("AbilityAttack missing attackClass");
  }
  attack.attackClass = model::attackClassFromString(json["attackClass"].get<String>());
  if (json.contains("dmg")) {
    attack.dmg = parseAbilityAttackDmg(json["dmg"]);
  }
  if (json.contains("save")) {
    attack.save = parseAbilitySave(json["save"]);
  }
  return attack;
}

model::AbilityStatus parseAbilityStatus(const Json& json) {
  model::AbilityStatus status;
  if (!json.contains("statusEffect") || !json["statusEffect"].is_string()) {
    throw std::runtime_error("AbilityStatus missing statusEffect");
  }
  status.statusEffect = json["statusEffect"].get<String>();
  if (json.contains("save")) {
    status.save = parseAbilitySave(json["save"]);
  }
  if (json.contains("baseDuration") && json["baseDuration"].is_number_integer()) {
    status.baseDuration = json["baseDuration"].get<int>();
  }
  if (json.contains("durationBonus") && json["durationBonus"].is_number_integer()) {
    status.durationBonus = json["durationBonus"].get<int>();
  }
  return status;
}

model::AbilityRestore parseAbilityRestore(const Json& json) {
  model::AbilityRestore restore;
  if (!json.contains("restoreWhich") || !json["restoreWhich"].is_string()) {
    throw std::runtime_error("AbilityRestore missing restoreWhich");
  }
  restore.restoreWhich = model::currentStatEnumFromString(json["restoreWhich"].get<String>());
  restore.restoreDice = parseDiceArray(json, "restoreDice");
  if (!json.contains("restoreBonus") || !json["restoreBonus"].is_number_integer()) {
    throw std::runtime_error("AbilityRestore missing restoreBonus");
  }
  restore.restoreBonus = json["restoreBonus"].get<int>();
  if (!json.contains("restoreStat") || !json["restoreStat"].is_string()) {
    throw std::runtime_error("AbilityRestore missing restoreStat");
  }
  restore.restoreStat = model::statsEnumFromString(json["restoreStat"].get<String>());
  if (!json.contains("restoreStatMult") || !json["restoreStatMult"].is_number_integer()) {
    throw std::runtime_error("AbilityRestore missing restoreStatMult");
  }
  restore.restoreStatMult = json["restoreStatMult"].get<int>();
  return restore;
}

model::AbilityDepiction parseAbilityDepiction(const Json& json) {
  model::AbilityDepiction depiction;
  if (!json.contains("dmgAnim") || !json["dmgAnim"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing dmgAnim");
  }
  depiction.dmgAnim = json["dmgAnim"].get<String>();
  if (json.contains("projectileType") && json["projectileType"].is_string()) {
    depiction.projectileType = model::projectileTypeFromString(json["projectileType"].get<String>());
  } else if (json.contains("projectileAnim") && json["projectileAnim"].is_string()) {
    depiction.projectileType = model::projectileTypeFromAnimName(json["projectileAnim"].get<String>());
  } else {
    throw std::runtime_error("AbilityDepiction missing projectileType");
  }
  if (!json.contains("projectilePath") || !json["projectilePath"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing projectilePath");
  }
  depiction.projectilePath = model::projectilePathFromString(json["projectilePath"].get<String>());
  if (!json.contains("startSound") || !json["startSound"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing startSound");
  }
  depiction.startSound = json["startSound"].get<String>();
  if (!json.contains("dmgSound") || !json["dmgSound"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing dmgSound");
  }
  depiction.dmgSound = json["dmgSound"].get<String>();
  return depiction;
}

} // namespace db
