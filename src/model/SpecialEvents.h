#pragma once

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace model {

enum class GameEventType { MODAL, TALK };

enum class GameEventChildType { KEYWORD, CHOICE, END, EXEC, BOOL, SWITCH };

enum class KeywordType { K, K_DUP, K_SWITCH, K_CHILD };

struct VariableValue {
  std::string str;
  std::optional<std::string> evaluated;
};

struct KeywordDataK {
  KeywordType keywordType = KeywordType::K;
  std::string text;
};

struct KeywordDataKDup {
  KeywordType keywordType = KeywordType::K_DUP;
  std::string keyword;
};

struct KeywordCheck {
  std::string conditionStr;
  std::string next;
};

struct KeywordDataKSwitch {
  KeywordType keywordType = KeywordType::K_SWITCH;
  std::string defaultNext; // id of next child
  std::vector<KeywordCheck> checks;
};

struct KeywordDataKChild {
  KeywordType keywordType = KeywordType::K_CHILD;
  std::string next; // id of next child
};

// Discriminated union for KeywordData
struct KeywordData {
  KeywordType keywordType;
  // Union of possible data - only one should be populated based on keywordType
  std::optional<KeywordDataK> k;
  std::optional<KeywordDataKDup> kDup;
  std::optional<KeywordDataKSwitch> kSwitch;
  std::optional<KeywordDataKChild> kChild;
};

struct Choice {
  std::string text;
  std::optional<std::string> conditionStr; // string that represents a condition to evaluate
  std::string next;
};

struct GameEventChildKeyword {
  GameEventChildType eventChildType = GameEventChildType::KEYWORD;
  std::string id;
  std::map<std::string, KeywordData> keywords;
};

struct GameEventChildChoice {
  GameEventChildType eventChildType = GameEventChildType::CHOICE;
  std::string id;
  std::string text;
  std::vector<Choice> choices;
};

struct SwitchCase {
  std::string conditionStr;
  std::string next;
};

struct GameEventChildSwitch {
  GameEventChildType eventChildType = GameEventChildType::SWITCH;
  std::string id;
  std::string switchStr;
  std::vector<SwitchCase> cases;
};

struct GameEventChildExec {
  GameEventChildType eventChildType = GameEventChildType::EXEC;
  std::string id;
  std::string text;
  std::string execStr;
  std::string next;
};

struct GameEventChildBool {
  GameEventChildType eventChildType = GameEventChildType::BOOL;
  std::string id;
  std::string checkStr;
  std::string pass; // id of next child
  std::string fail; // id of next child
};

struct GameEventChildEnd {
  GameEventChildType eventChildType = GameEventChildType::END;
  std::string id;
  std::string next;
};

// Discriminated union for GameEventChild
using GameEventChild = std::variant<GameEventChildKeyword, GameEventChildChoice,
                                     GameEventChildSwitch, GameEventChildExec,
                                     GameEventChildBool, GameEventChildEnd>;

struct GameEvent {
  std::string id;
  std::string title;
  GameEventType eventType; // indicates which ui layer to use for the event
  std::string icon; // name of sprite to use for the event
  std::map<std::string, VariableValue> vars; // a mapping from variable name to its original text and it's evaluated value
  std::vector<GameEventChild> children;
};

} // namespace model

