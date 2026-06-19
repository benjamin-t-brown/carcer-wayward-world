#include "LoadAbilityJson.h"
#include <stdexcept>

namespace db {

std::vector<model::Dice> parseDiceArray(const nlohmann::json& json,
                                        const std::string& fieldName) {
  if (!json.contains(fieldName) || !json[fieldName].is_array()) {
    throw std::runtime_error("Missing or invalid field: " + fieldName);
  }
  std::vector<model::Dice> dice;
  for (const auto& dieJson : json[fieldName]) {
    if (!dieJson.is_string()) {
      throw std::runtime_error("Dice entry must be a string in " + fieldName);
    }
    dice.push_back(model::diceFromString(dieJson.get<std::string>()));
  }
  return dice;
}

model::TargetSelectInfo parseTargetSelectInfo(const nlohmann::json& json) {
  model::TargetSelectInfo info;
  if (!json.contains("targetType") || !json["targetType"].is_string()) {
    throw std::runtime_error("targetSelect missing targetType");
  }
  info.targetType = model::targetSelectTypeFromString(json["targetType"]);
  if (!json.contains("allegianceSelectType") || !json["allegianceSelectType"].is_string()) {
    throw std::runtime_error("targetSelect missing allegianceSelectType");
  }
  info.allegianceSelectType =
      model::targetAllegianceSelectTypeFromString(json["allegianceSelectType"]);
  if (!json.contains("numTargetableUnits") || !json["numTargetableUnits"].is_number_integer()) {
    throw std::runtime_error("targetSelect missing numTargetableUnits");
  }
  info.numTargetableUnits = json["numTargetableUnits"];
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
  info.zoneSize.x = zoneSize["x"];
  info.zoneSize.y = zoneSize["y"];
  if (!json.contains("range") || !json["range"].is_number_integer()) {
    throw std::runtime_error("targetSelect missing range");
  }
  info.range = json["range"];
  return info;
}

model::Stats parseStats(const nlohmann::json& json) {
  model::Stats stats;
  if (json.contains("STR") && json["STR"].is_number_integer()) {
    stats.STR = json["STR"];
  }
  if (json.contains("MND") && json["MND"].is_number_integer()) {
    stats.MND = json["MND"];
  }
  if (json.contains("CON") && json["CON"].is_number_integer()) {
    stats.CON = json["CON"];
  }
  if (json.contains("AGI") && json["AGI"].is_number_integer()) {
    stats.AGI = json["AGI"];
  }
  if (json.contains("LCK") && json["LCK"].is_number_integer()) {
    stats.LCK = json["LCK"];
  }
  return stats;
}

model::CurrentStats parseCurrentStats(const nlohmann::json& json) {
  model::CurrentStats stats;
  if (json.contains("HP") && json["HP"].is_number_integer()) {
    stats.HP = json["HP"];
  }
  if (json.contains("AP") && json["AP"].is_number_integer()) {
    stats.AP = json["AP"];
  }
  if (json.contains("MANA") && json["MANA"].is_number_integer()) {
    stats.MANA = json["MANA"];
  }
  if (json.contains("AC") && json["AC"].is_number_integer()) {
    stats.AC = json["AC"];
  }
  return stats;
}

model::Resistance parseResistance(const nlohmann::json& json) {
  model::Resistance resistance;
  if (!json.contains("attackType") || !json["attackType"].is_string()) {
    throw std::runtime_error("Resistance missing attackType");
  }
  resistance.attackType = model::damageTypeFromString(json["attackType"]);
  if (!json.contains("mod") || !json["mod"].is_number_integer()) {
    throw std::runtime_error("Resistance missing mod");
  }
  resistance.mod = json["mod"];
  return resistance;
}

model::AbilitySave parseAbilitySave(const nlohmann::json& json) {
  model::AbilitySave save;
  if (!json.contains("saveStat") || !json["saveStat"].is_string()) {
    throw std::runtime_error("AbilitySave missing saveStat");
  }
  save.saveStat = model::statsEnumFromString(json["saveStat"]);
  if (!json.contains("saveBase") || !json["saveBase"].is_number_integer()) {
    throw std::runtime_error("AbilitySave missing saveBase");
  }
  save.saveBase = json["saveBase"];
  if (!json.contains("saveAgainst") || !json["saveAgainst"].is_string()) {
    throw std::runtime_error("AbilitySave missing saveAgainst");
  }
  save.saveAgainst = model::statsEnumFromString(json["saveAgainst"]);
  if (!json.contains("saveAgainstBase") || !json["saveAgainstBase"].is_number_integer()) {
    throw std::runtime_error("AbilitySave missing saveAgainstBase");
  }
  save.saveAgainstBase = json["saveAgainstBase"];
  return save;
}

model::AbilityAttackDmg parseAbilityAttackDmg(const nlohmann::json& json) {
  model::AbilityAttackDmg dmg;
  dmg.dmgDice = parseDiceArray(json, "dmgDice");
  if (!json.contains("dmgBonus") || !json["dmgBonus"].is_number_integer()) {
    throw std::runtime_error("AbilityAttackDmg missing dmgBonus");
  }
  dmg.dmgBonus = json["dmgBonus"];
  if (!json.contains("dmgStat") || !json["dmgStat"].is_string()) {
    throw std::runtime_error("AbilityAttackDmg missing dmgStat");
  }
  dmg.dmgStat = model::statsEnumFromString(json["dmgStat"]);
  if (!json.contains("dmgStatMult") || !json["dmgStatMult"].is_number_integer()) {
    throw std::runtime_error("AbilityAttackDmg missing dmgStatMult");
  }
  dmg.dmgStatMult = json["dmgStatMult"];
  if (!json.contains("attackBonus") || !json["attackBonus"].is_number_integer()) {
    throw std::runtime_error("AbilityAttackDmg missing attackBonus");
  }
  dmg.attackBonus = json["attackBonus"];
  return dmg;
}

model::AbilityAttack parseAbilityAttack(const nlohmann::json& json) {
  model::AbilityAttack attack;
  if (!json.contains("attackClass") || !json["attackClass"].is_string()) {
    throw std::runtime_error("AbilityAttack missing attackClass");
  }
  attack.attackClass = model::attackClassFromString(json["attackClass"]);
  if (json.contains("dmg")) {
    attack.dmg = parseAbilityAttackDmg(json["dmg"]);
  }
  if (json.contains("save")) {
    attack.save = parseAbilitySave(json["save"]);
  }
  return attack;
}

model::AbilityStatus parseAbilityStatus(const nlohmann::json& json) {
  model::AbilityStatus status;
  if (!json.contains("statusEffect") || !json["statusEffect"].is_string()) {
    throw std::runtime_error("AbilityStatus missing statusEffect");
  }
  status.statusEffect = json["statusEffect"];
  if (json.contains("save")) {
    status.save = parseAbilitySave(json["save"]);
  }
  if (json.contains("baseDuration") && json["baseDuration"].is_number_integer()) {
    status.baseDuration = json["baseDuration"];
  }
  if (json.contains("durationBonus") && json["durationBonus"].is_number_integer()) {
    status.durationBonus = json["durationBonus"];
  }
  return status;
}

model::AbilityRestore parseAbilityRestore(const nlohmann::json& json) {
  model::AbilityRestore restore;
  if (!json.contains("restoreWhich") || !json["restoreWhich"].is_string()) {
    throw std::runtime_error("AbilityRestore missing restoreWhich");
  }
  restore.restoreWhich = model::currentStatEnumFromString(json["restoreWhich"]);
  restore.restoreDice = parseDiceArray(json, "restoreDice");
  if (!json.contains("restoreBonus") || !json["restoreBonus"].is_number_integer()) {
    throw std::runtime_error("AbilityRestore missing restoreBonus");
  }
  restore.restoreBonus = json["restoreBonus"];
  if (!json.contains("restoreStat") || !json["restoreStat"].is_string()) {
    throw std::runtime_error("AbilityRestore missing restoreStat");
  }
  restore.restoreStat = model::statsEnumFromString(json["restoreStat"]);
  if (!json.contains("restoreStatMult") || !json["restoreStatMult"].is_number_integer()) {
    throw std::runtime_error("AbilityRestore missing restoreStatMult");
  }
  restore.restoreStatMult = json["restoreStatMult"];
  return restore;
}

model::AbilityDepiction parseAbilityDepiction(const nlohmann::json& json) {
  model::AbilityDepiction depiction;
  if (!json.contains("dmgAnim") || !json["dmgAnim"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing dmgAnim");
  }
  depiction.dmgAnim = json["dmgAnim"];
  if (json.contains("projectileType") && json["projectileType"].is_string()) {
    depiction.projectileType = model::projectileTypeFromString(json["projectileType"]);
  } else if (json.contains("projectileAnim") && json["projectileAnim"].is_string()) {
    depiction.projectileType = model::projectileTypeFromAnimName(json["projectileAnim"]);
  } else {
    throw std::runtime_error("AbilityDepiction missing projectileType");
  }
  if (!json.contains("projectilePath") || !json["projectilePath"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing projectilePath");
  }
  depiction.projectilePath = model::projectilePathFromString(json["projectilePath"]);
  if (!json.contains("startSound") || !json["startSound"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing startSound");
  }
  depiction.startSound = json["startSound"];
  if (!json.contains("dmgSound") || !json["dmgSound"].is_string()) {
    throw std::runtime_error("AbilityDepiction missing dmgSound");
  }
  depiction.dmgSound = json["dmgSound"];
  return depiction;
}

} // namespace db
