#include "lib/Json.h"
#include "lib/sdl2w/Logger.h"

namespace {

bool assertEqual(const String& actual, const String& expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected.cStr() << "' but got '" << actual.cStr() << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(const String& actual, const char* expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected << "' but got '" << actual.cStr() << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestJson" << LOG_ENDL;

  try {
    bool ok = true;

    const Json parsed =
        Json::parse(R"({"name":"sword","value":10})", nullptr, true, true);
    ok = parsed.is_object() && ok;
    ok = parsed["name"].is_string() && ok;
    ok = assertEqual(parsed["name"].get<String>(), "sword", "parsed.name") && ok;
    ok = assertEqual(parsed["value"].get<int>(), 10, "parsed.value") && ok;

    Json built;
    built["label"] = String("potion");
    ok = assertEqual(built["label"].get<String>(), "potion", "built.label") && ok;

    ok = assertEqual(parsed.value("missing", String()), String(), "value.string_default") && ok;
    ok = assertEqual(parsed.value("value", 0), 10, "value.int_present") && ok;

    const Json tilesDoc =
        Json::parse(R"({"tiles":{"0":[1,2],"1":[3]}})", nullptr, true, true);
    int tileLayerCount = 0;
    for (const auto& [layerKey, graphicsJson] : tilesDoc["tiles"].items()) {
      ++tileLayerCount;
      ok = graphicsJson.is_array() && ok;
      if (layerKey == "0") {
        ok = assertEqual(static_cast<int>(graphicsJson.size()), 2, "tiles[0].size") && ok;
        ok = assertEqual(graphicsJson[0].get<int>(), 1, "tiles[0][0]") && ok;
      } else if (layerKey == "1") {
        ok = assertEqual(static_cast<int>(graphicsJson.size()), 1, "tiles[1].size") && ok;
        ok = assertEqual(graphicsJson[0].get<int>(), 3, "tiles[1][0]") && ok;
      }
    }
    ok = assertEqual(tileLayerCount, 2, "tiles.layer_count") && ok;

    if (!ok) {
      return 1;
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in TestJson: " << e.what() << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestJson passed" << LOG_ENDL;
  return 0;
}
