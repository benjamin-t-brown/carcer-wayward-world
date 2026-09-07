#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
import carcer;
import sdl2w;
import bmin.string_interop;
#include "macros.h"
#include "../../setupTestUi.h"

namespace {

ui::PageMagicSetupSpellEntry makeSpellEntryFromDb(const db::Database& database,
                                                  const model::SpellTemplate& spell) {
  ui::PageMagicSetupSpellEntry entry;
  entry.id = spell.name;
  const auto* ability =
      database.findAbilityTemplate(bmin::toStringView(spell.abilityName));
  entry.label = spell.label;
  if (entry.label.empty() && ability != nullptr) {
    entry.label = ability->label;
  }
  if (entry.label.empty()) {
    entry.label = spell.name;
  }
  entry.iconSprite = spell.icon;
  if (entry.iconSprite.empty() && ability != nullptr) {
    entry.iconSprite = ability->icon;
  }
  entry.manaCost = model::spellAbilityManaCost(spell, database);
  for (const auto& req : spell.requiredRunes) {
    const auto sprite = model::runeTypeToSpriteName(req.type);
    const int count = req.count > 0 ? req.count : 1;
    for (int i = 0; i < count; ++i) {
      entry.requiredRuneSprites.push_back(sprite);
    }
  }
  return entry;
}

ui::PageMagicSetupProps makeMagicSetupFixtureProps(
    int windowWidth,
    int windowHeight,
    const model::Player& player,
    const model::CharacterPlayer& characterPlayer,
    const db::Database& database) {
  ui::PageMagicSetupProps pageProps;
  pageProps.width = windowWidth;
  pageProps.height = windowHeight;
  pageProps.characterPlayerId = characterPlayer.instanceId;
  pageProps.characterPlayerLabel = characterPlayer.params.label;
  pageProps.characterPlayerSprite = model::characterPlayerGetSprite(characterPlayer);
  pageProps.partyMemberMagicIndex = player.currentPartyMemberMagicIndex;
  for (const auto& member : player.party) {
    pageProps.partyMembers.pushBack(
        {.spriteName = model::characterPlayerGetSprite(member)});
  }

  // Dense equipped list from live character state.
  pageProps.runeSlots.clear();
  for (size_t i = 0; i < model::CharacterPlayer::kRuneSlotCount; ++i) {
    if (i < characterPlayer.equippedRunes.size()) {
      pageProps.runeSlots.pushBack(ui::PageMagicSetupRuneSlot{
          .filled = true,
          .iconSprite =
              model::runeTypeToSpriteName(characterPlayer.equippedRunes[i]),
      });
    } else {
      pageProps.runeSlots.pushBack(ui::PageMagicSetupRuneSlot{.filled = false});
    }
  }
  pageProps.selectedRuneSlotIndex = -1;

  pageProps.elementCounts.clear();
  for (int i = 0; i < model::kRuneTypeCount; ++i) {
    const auto runeType = model::runeTypeFromIndex(i);
    const int available = model::characterPlayerCountAvailableRunesOfType(
        characterPlayer, runeType);
    const int equipped = model::characterPlayerCountEquippedRunesOfType(
        characterPlayer, runeType);
    pageProps.elementCounts.pushBack(ui::PageMagicSetupElementCount{
        .iconSprite = model::runeTypeToSpriteName(runeType),
        // Remaining unequipped (same as Equip Runes modal center counts).
        .count = available > equipped ? available - equipped : 0,
    });
  }

  // Single list from DB spells; ready iff equipped runes meet requiredRunes.
  model::CharacterPlayer runeCheckCharacter = characterPlayer;
  const bmin::DynArray<bmin::String> dbSpellNames = {
      "FLAME",
      "HEAL_SELF_MINOR",
      "SINGE",
  };
  for (const auto& spellName : dbSpellNames) {
    const auto* spell = database.findSpellTemplate(bmin::toStringView(spellName));
    if (spell == nullptr) {
      LOG(WARN) << "TestPageMagicSetup: missing spell template " << spellName
                << LOG_ENDL;
      continue;
    }
    runeCheckCharacter.knownSpells.pushBack(spellName);
    auto entry = makeSpellEntryFromDb(database, *spell);
    int equippedCounts[model::kRuneTypeCount] = {};
    for (const auto& runeType : runeCheckCharacter.equippedRunes) {
      equippedCounts[model::runeTypeIndex(runeType)]++;
    }
    entry.ready = true;
    for (const auto& requiredRune : spell->requiredRunes) {
      if (equippedCounts[model::runeTypeIndex(requiredRune.type)] < requiredRune.count) {
        entry.ready = false;
        break;
      }
    }
    pageProps.spells.pushBack(entry);
  }

  return pageProps;
}

} // namespace

class TestLayer : public layers::Layer {
  db::Database* database = nullptr;

  void syncFromCharacter() {
    auto* page = getUiElement<ui::PageMagicSetup>("pageMagicSetup");
    if (page == nullptr || database == nullptr || !hasStateManager()) {
      return;
    }
    auto& player = getStateManager()->getState().player;
    auto* characterPlayer = model::playerFindPartyMemberByIndex(
        player, player.currentPartyMemberMagicIndex);
    if (characterPlayer == nullptr) {
      return;
    }
    auto [windowWidth, windowHeight] = window->getDims();
    page->setProps(makeMagicSetupFixtureProps(
        windowWidth, windowHeight, player, *characterPlayer, *database));
  }

public:
  static constexpr std::string_view LAYER_ID = "test_layer_magic_setup";

  TestLayer(sdl2w::Window* _window,
            db::Database* _database,
            ui::PageMagicSetupProps pageProps)
      : layers::Layer(_window, LAYER_ID), database(_database) {
    auto pageMagicSetup = bmin::makeUnique<ui::PageMagicSetup>(window);
    pageMagicSetup->setId("pageMagicSetup");
    pageMagicSetup->setPos(0, 0);
    pageMagicSetup->setProps(pageProps);
    addUiElement(pageMagicSetup.release());

    // Match LayerMagic: refresh page when selection or equip editor changes.
    subscribeAction<state::ActionEvent::UiSetCurrentPartyMemberMagic>(
        [this](auto&, auto&) { syncFromCharacter(); });
    subscribeAction<state::ActionEvent::UiCommitEquipRunes>(
        [this](auto&, auto&) { syncFromCharacter(); });
    subscribeAction<state::ActionEvent::UiCancelEquipRunes>(
        [this](auto&, auto&) { syncFromCharacter(); });
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start PageMagicSetup test" << LOG_ENDL;
  srand(time(NULL));

  bmin::UniquePtr<layers::LayerManager> layerManager;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PageMagicSetup test initialized" << LOG_ENDL;

    bmin::DynArray<bmin::String> partyTemplateNames = {
        "testPartyMember1",
        "testPartyMember2",
        "testPartyMember3",
        "testPartyMember4",
    };

    auto& player = stateManager.getState().player;
    for (const auto& templateName : partyTemplateNames) {
      player.party.pushBack(model::CharacterPlayer(
          database.getCharacterTemplate(bmin::toStringView(templateName))));
    }
    player.currentPartyMemberMagicIndex = 0;
    // Seed distinct rune loads so switching party members visibly changes UI.
    auto& member0 = player.party[0];
    member0.equippedRunes = {
        model::RuneType::HEAT,
        model::RuneType::HEAT,
        model::RuneType::HEAT,
        model::RuneType::ENTROPY,
        model::RuneType::REGROWTH,
        model::RuneType::DISPLACE,
    };
    model::characterPlayerSetAvailableRuneCount(member0, model::RuneType::HEAT, 3);
    model::characterPlayerSetAvailableRuneCount(member0, model::RuneType::ENTROPY, 1);
    model::characterPlayerSetAvailableRuneCount(
        member0, model::RuneType::REGROWTH, 12);
    model::characterPlayerSetAvailableRuneCount(
        member0, model::RuneType::DISPLACE, 1);
    model::characterPlayerSetAvailableRuneCount(member0, model::RuneType::EXPAND, 7);
    model::characterPlayerSetAvailableRuneCount(member0, model::RuneType::ATTACH, 2);
    model::characterPlayerSetAvailableRuneCount(
        member0, model::RuneType::TRANSFORM, 5);
    model::characterPlayerSetAvailableRuneCount(member0, model::RuneType::COMPACT, 9);

    if (player.party.size() > 1) {
      auto& member1 = player.party[1];
      member1.equippedRunes = {
          model::RuneType::EXPAND,
          model::RuneType::EXPAND,
          model::RuneType::ATTACH,
      };
      model::characterPlayerSetAvailableRuneCount(
          member1, model::RuneType::EXPAND, 4);
      model::characterPlayerSetAvailableRuneCount(
          member1, model::RuneType::ATTACH, 2);
      model::characterPlayerSetAvailableRuneCount(
          member1, model::RuneType::COMPACT, 3);
    }
    if (player.party.size() > 2) {
      auto& member2 = player.party[2];
      member2.equippedRunes = {model::RuneType::TRANSFORM};
      model::characterPlayerSetAvailableRuneCount(
          member2, model::RuneType::TRANSFORM, 5);
      model::characterPlayerSetAvailableRuneCount(
          member2, model::RuneType::HEAT, 1);
    }

    auto [windowWidth, windowHeight] = window.getDims();
    auto pageProps = makeMagicSetupFixtureProps(
        windowWidth, windowHeight, player, member0, database);

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    auto* testLayer = new TestLayer(&window, &database, std::move(pageProps));
    layerManager->addLayer(testLayer);
    // Match production open/close: baseline layer must be on the events stack.
    layerManager->moveToFront(testLayer);

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          layerManager->handleMouseDown(x, y, button);
        });
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
          layerManager->handleMouseUp(x, y, button);
        });

    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL, [&](int x, int y, int delta) {
          layerManager->handleMouseWheel(x, y, delta);
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    // Process UI actions before layer cleanup so closed layers restore the
    // previous front in the same frame they are marked for removal.
    stateManager.update(window.getDeltaTime());
    layerManager->update(window.getDeltaTime());
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.setBackgroundColor({100, 100, 100, 255});
    draw.clearScreen();

    layerManager->render(window.getDeltaTime());
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{640, 480, "PageMagicSetup Test"},
              _init,
              _updateRender,
              [&]() { layerManager.reset(); });
  LOG(INFO) << "End PageMagicSetup test" << LOG_ENDL;
  return 0;
}
