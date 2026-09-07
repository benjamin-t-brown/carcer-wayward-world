module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <vector>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>

module carcer.ui.screens;
#include "macros.h"

namespace ui {

PageCharacter::PageCharacter(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageCharacter::setProps(const PageCharacterProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageCharacterProps& PageCharacter::getProps() { return props; }

const PageCharacterProps& PageCharacter::getProps() const { return props; }

const std::pair<int, int> PageCharacter::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

UiElement*
PageCharacter::buildStatSection(const PageCharacterStatRowSectionArgs& sectionProps) {

  auto [title,
        rows,
        sectionWidth,
        y,
        showModButtons,
        buttonMinusDisabled,
        buttonPlusDisabled] = sectionProps;
  const int headerBottomMargin = 4;
  const int headerPadding = 4;
  const int linePadding = 2;

  auto* section = new Quad(window, this);
  section->setId("quad_" + title);

  auto* header = new TextLine(window, section);
  header->setId("title_" + title);
  TextFontProps headerFont;
  setBaseFontConfig(headerFont, BaseFontConfig::MODAL_TITLE);
  header->setPos(headerPadding * style.scale, headerPadding * style.scale);
  header->setScale(1.f);
  TextLineProps headerProps;
  headerProps.fontFamily = headerFont.fontFamily;
  headerProps.fontSize = sdl2w::TEXT_SIZE_24;
  headerProps.fontColor = Colors::DarkBlue;
  headerProps.textAlign = TextAlign::LEFT_TOP;
  headerProps.textBlocks.pushBack(TextBlock{.text = title});
  header->setProps(headerProps);
  section->addChild(header);

  auto [headerW, headerH] = header->getDims();

  auto* list = new VerticalList(window, section);
  list->setId("list_" + title);
  list->setPos(linePadding * style.scale, headerH + headerBottomMargin * style.scale);
  list->setScale(style.scale);
  list->setProps(VerticalListProps{
      .width = sectionWidth - 2 * linePadding,
      .lineHeight = 32,
      .lineGap = 0,
  });
  for (const auto& row : rows) {
    auto* statRow = new Quad(window, list);
    statRow->setId("row_" + row.label);
    statRow->setPos(0, 0);
    statRow->setScale(style.scale);

    const int buttonSize = 32;
    const int rowWidth = sectionWidth - 2 * linePadding;
    int leftX = -4;
    int buttonX = rowWidth - linePadding;

    auto* buttonHelp = new ButtonModal(window, statRow);
    buttonHelp->setId("button_help_" + row.label);
    buttonHelp->setPos(leftX * style.scale, 0);
    buttonHelp->setScale(style.scale);
    buttonHelp->setProps(ButtonModalProps{
        .text = "?",
        .width = buttonSize,
        .height = buttonSize,
        .bgColor = Colors::Transparent,
        .bgColorTopRight = Colors::Transparent,
        .bgColorBottomLeft = Colors::Transparent,
        .fontSize = sdl2w::TEXT_SIZE_14,
        .fontColor = Colors::Grey,
    });
    if (!row.helpDescription.empty()) {
      buttonHelp->addEventObserver(
          new ObserverShowLayerPopupText(window, row.label, row.helpDescription));
    }
    statRow->addChild(buttonHelp);
    leftX += buttonSize + linePadding;

    auto* statLine = new TextLine(window, list);
    statLine->setId("item_" + row.label);
    TextFontProps statFont;
    setBaseFontConfig(statFont, BaseFontConfig::MODAL_TEXT);
    const bmin::String statValueText =
        row.valueText.empty() ? bmin::toString(row.value) : row.valueText;
    TextLineProps rowProps;
    rowProps.fontFamily = statFont.fontFamily;
    rowProps.fontSize = sdl2w::TEXT_SIZE_20;
    rowProps.fontColor = Colors::Black;
    rowProps.textAlign = TextAlign::LEFT_CENTER;
    rowProps.textBlocks.pushBack(TextBlock{.text = row.label + ": " + statValueText});
    statLine->setScale(1.f);
    statLine->setProps(rowProps);
    statLine->setPos(leftX * style.scale, list->getProps().lineHeight * style.scale / 2);
    statRow->addChild(statLine);

    if (showModButtons) {
      buttonX -= buttonSize;
      auto* buttonPlus = new ButtonIcon(window, statRow);
      buttonPlus->setId("button_plus_" + row.label);
      buttonPlus->setPos(buttonX * style.scale, 0);
      buttonPlus->setScale(style.scale);
      buttonPlus->setProps(ButtonIconProps{.regularSprite = ButtonIcon::PLUS_ICON1,
                                           .activeSprite = ButtonIcon::PLUS_ICON2,
                                           .iconSize = buttonSize,
                                           .isDisabled = buttonPlusDisabled});
      statRow->addChild(buttonPlus);

      buttonX -= buttonSize;
      auto* buttonMinus = new ButtonIcon(window, statRow);
      buttonMinus->setId("button_minus_" + row.label);
      buttonMinus->setPos(buttonX * style.scale, 0);
      buttonMinus->setScale(style.scale);
      buttonMinus->setProps(ButtonIconProps{.regularSprite = ButtonIcon::MINUS_ICON1,
                                            .activeSprite = ButtonIcon::MINUS_ICON2,
                                            .iconSize = buttonSize,
                                            .isDisabled = buttonMinusDisabled});
      statRow->addChild(buttonMinus);
    }

    statRow->setProps(QuadProps{
        .width = sectionWidth - 2 * linePadding,
        .height = list->getProps().lineHeight,
        .bgColor = Colors::Transparent,
    });
    list->addListItem(statRow);
  }
  list->build();
  section->addChild(list);
  auto [listW, listH] = list->getDims();

  section->setPos(linePadding * style.scale, y);
  section->setScale(1.f);
  section->setProps(QuadProps{
      .width = static_cast<int>(sectionWidth - 2 * linePadding * style.scale),
      .height = static_cast<int>(headerH + headerBottomMargin * style.scale + listH +
                                 linePadding * style.scale),
      .bgColor = Colors::Transparent,
  });

  return section;
}

void PageCharacter::addDerivedStatSections(
    SectionScrollable* scrollable,
    int sectionContentW,
    int& yAgg,
    const model::CharacterDerivedStats& derivedStats) {
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterDerivedStatDefinitions::derivedTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::hpLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::hpDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::hpValue(derivedStats)},
                  PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::manaLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::manaDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::manaValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::actionPointsLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::actionPointsDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::actionPointsValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::mightDamageLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::mightDamageDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::mightDamageValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::magicDamageLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::magicDamageDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::magicDamageValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::attackHitChanceLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::attackHitChanceDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::attackHitChanceValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::abilityPowerLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::abilityPowerDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::abilityPowerValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::damageReductionLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::damageReductionDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::damageReductionValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::armorClassLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::armorClassDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::armorClassValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::spellPotencyLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::spellPotencyDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::spellPotencyValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::resistancesLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::resistancesDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::resistancesValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::jumpDistanceLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::jumpDistanceDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::jumpDistanceValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::healingEffectivenessLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::healingEffectivenessDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::healingEffectivenessValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::statusEffectShieldLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::statusEffectShieldDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::statusEffectShieldValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::materiaSlotsLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::materiaSlotsDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::materiaSlotsValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::shieldBonusLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::shieldBonusDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::shieldBonusValue(derivedStats)},
            },
        // clang-format on
        .width = sectionContentW,
        .y = yAgg,
        .showModButtons = false,
        .buttonMinusDisabled = true,
        .buttonPlusDisabled = true,
    });
    scrollable->addChild(section);
    yAgg += section->getDims().second;
  }
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterDerivedStatDefinitions::derivedSkillsTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::enemyVisionRangeLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::enemyVisionRangeDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::enemyVisionRangeValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::mageLoreLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::mageLoreDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::mageLoreValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::toolUseLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::toolUseDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::toolUseValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::tradeDiscountLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::tradeDiscountDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::tradeDiscountValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::itemUsageLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::itemUsageDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::itemUsageValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::foodConsumptionLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::foodConsumptionDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::foodConsumptionValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::firstAidLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::firstAidDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::firstAidValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::ingredientFindChanceLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::ingredientFindChanceDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::ingredientFindChanceValue(derivedStats)},
                PageCharacterStatRowEntry{
                    .label = model::CharacterDerivedStatDefinitions::foodConsumptionPerDayLabel(),
                    .helpDescription = model::CharacterDerivedStatDefinitions::foodConsumptionPerDayDescription(),
                    .valueText = model::CharacterDerivedStatDefinitions::foodConsumptionPerDayValue(derivedStats)},
            },
        // clang-format on
        .width = sectionContentW,
        .y = yAgg,
        .showModButtons = false,
        .buttonMinusDisabled = true,
        .buttonPlusDisabled = true,
    });
    scrollable->addChild(section);
    yAgg += section->getDims().second;
  }
}

void PageCharacter::build() {
  children.clear();

  const model::CharacterStats emptyStats{};
  const model::CharacterStats& stats =
      props.characterPlayer ? props.characterPlayer->stats : emptyStats;
  const int characterLevel = 1;
  const model::CharacterDerivedStats derivedStats =
      model::computeCharacterDerivedStats(stats, characterLevel);

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto* modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);

  ModalStandardProps modalProps;
  modalProps.width = style.width;
  modalProps.height = style.height;
  if (props.characterPlayer) {
    modalProps.iconSprite = model::characterPlayerGetSprite(*props.characterPlayer);
  }
  modal->setProps(modalProps);
  syncHostStyleToCappedCentered(style);
  addChild(modal);

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  const int unscaledContentW = static_cast<int>(contentW / style.scale);
  const int unscaledContentH = static_cast<int>(contentH / style.scale);
  constexpr int MIN_SCROLLABLE_SECTION_WIDTH = 300;
  const bool useSingleColumn = (unscaledContentW / 2) < MIN_SCROLLABLE_SECTION_WIDTH;
  const int scrollableSectionWidth =
      useSingleColumn ? unscaledContentW : unscaledContentW / 2;

  auto* title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  if (props.characterPlayer && !props.characterPlayer->name.empty()) {
    titleBlock.text = props.characterPlayer->name;
  } else if (props.characterPlayer && !props.characterPlayer->params.label.empty()) {
    titleBlock.text = props.characterPlayer->params.label;
  } else {
    titleBlock.text = "Character";
  }
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto* scrollableStatsSection = new SectionScrollable(window, modal);
  scrollableStatsSection->setId("scrollableSection");
  scrollableStatsSection->setPos(contentX, contentY);
  scrollableStatsSection->setScale(style.scale);
  scrollableStatsSection->setProps(SectionScrollableProps{
      .width = scrollableSectionWidth,
      .height = unscaledContentH,
      .scrollBarWidth = 32,
      .bgColor = Colors::White,
  });
  addChild(scrollableStatsSection);

  auto [scrollableContentW, scrollableContentH] =
      scrollableStatsSection->getContentDims();

  int yAgg = 0;
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = TRANSLATE("Level"),
        .rows = {PageCharacterStatRowEntry{
            .label = TRANSLATE("Level"),
            .helpDescription = TRANSLATE("Level up to increase stats and abilities."),
            .value = 1,
        }},
        .width = scrollableContentW,
        .y = yAgg,
        .showModButtons = false,
        .buttonMinusDisabled = true,
        .buttonPlusDisabled = true,
    });
    scrollableStatsSection->addChild(section);
    yAgg += section->getDims().second;
  }
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterStatDefinitions::attributesTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::strengthLabel(),
                    .helpDescription = model::CharacterStatDefinitions::strengthDescription(),
                    .value = stats.generic.str,
                },
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::agilityLabel(),
                    .helpDescription = model::CharacterStatDefinitions::agilityDescription(),
                    .value = stats.generic.agi},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::constitutionLabel(),
                    .helpDescription = model::CharacterStatDefinitions::constitutionDescription(),
                    .value = stats.generic.con},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::mindLabel(),
                    .helpDescription = model::CharacterStatDefinitions::mindDescription(),
                    .value = stats.generic.mnd},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::luckLabel(),
                    .helpDescription = model::CharacterStatDefinitions::luckDescription(),
                    .value = stats.generic.lck},
            },
        // clang-format on
        .width = scrollableContentW,
        .y = yAgg,
        .showModButtons = true,
        .buttonMinusDisabled = false,
        .buttonPlusDisabled = false,
    });
    scrollableStatsSection->addChild(section);
    yAgg += section->getDims().second;
  }
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterStatDefinitions::weaponMasteryTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::edgedWeaponsLabel(),
                    .helpDescription = model::CharacterStatDefinitions::edgedWeaponsDescription(),
                    .value = stats.trainable.weapon.edged},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::poleWeaponsLabel(),
                    .helpDescription = model::CharacterStatDefinitions::poleWeaponsDescription(),
                    .value = stats.trainable.weapon.pole},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::bluntWeaponsLabel(),
                    .helpDescription = model::CharacterStatDefinitions::bluntWeaponsDescription(),
                    .value = stats.trainable.weapon.blunt},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::rangeWeaponsLabel(),
                    .helpDescription = model::CharacterStatDefinitions::rangeWeaponsDescription(),
                    .value = stats.trainable.weapon.range},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::unarmedLabel(),
                    .helpDescription = model::CharacterStatDefinitions::unarmedDescription(),
                    .value = stats.trainable.weapon.unarmed},
            },
        // clang-format on
        .width = scrollableContentW,
        .y = yAgg,
        .showModButtons = true,
        .buttonMinusDisabled = false,
        .buttonPlusDisabled = false,
    });
    scrollableStatsSection->addChild(section);
    yAgg += section->getDims().second;
  }
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterStatDefinitions::magicMasteryTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::manaLabel(),
                    .helpDescription = model::CharacterStatDefinitions::manaDescription(),
                    .value = stats.trainable.magic.mana},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::abilityPowerLabel(),
                    .helpDescription = model::CharacterStatDefinitions::abilityPowerDescription(),
                    .value = stats.trainable.magic.abilityPower},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::attunementLabel(),
                    .helpDescription = model::CharacterStatDefinitions::attunementDescription(),
                    .value = stats.trainable.magic.attunement},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::faithLabel(),
                    .helpDescription = model::CharacterStatDefinitions::faithDescription(),
                    .value = stats.trainable.magic.faith},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::loreLabel(),
                    .helpDescription = model::CharacterStatDefinitions::loreDescription(),
                    .value = stats.trainable.magic.lore},
            },
        // clang-format on
        .width = scrollableContentW,
        .y = yAgg,
        .showModButtons = true,
        .buttonMinusDisabled = false,
        .buttonPlusDisabled = false,
    });
    scrollableStatsSection->addChild(section);
    yAgg += section->getDims().second;
  }
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterStatDefinitions::bodyMasteryTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::resistPhysicalLabel(),
                    .helpDescription = model::CharacterStatDefinitions::resistPhysicalDescription(),
                    .value = stats.trainable.body.resistPhysical},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::resistMagicalLabel(),
                    .helpDescription = model::CharacterStatDefinitions::resistMagicalDescription(),
                    .value = stats.trainable.body.resistMagical},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::healingEffLabel(),
                    .helpDescription = model::CharacterStatDefinitions::healingEffDescription(),
                    .value = stats.trainable.body.healingEffectiveness},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::damageReductionLabel(),
                    .helpDescription = model::CharacterStatDefinitions::damageReductionDescription(),
                    .value = stats.trainable.body.dr},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::armorTrainingLabel(),
                    .helpDescription = model::CharacterStatDefinitions::armorTrainingDescription(),
                    .value = stats.trainable.body.armorTraining},
            },
        // clang-format on
        .width = scrollableContentW,
        .y = yAgg,
        .showModButtons = true,
        .buttonMinusDisabled = false,
        .buttonPlusDisabled = false,
    });
    scrollableStatsSection->addChild(section);
    yAgg += section->getDims().second;
  }
  {
    auto* section = buildStatSection(PageCharacterStatRowSectionArgs{
        .title = model::CharacterStatDefinitions::skillsTitle(),
        // clang-format off
            .rows = {
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::trickeryLabel(),
                    .helpDescription = model::CharacterStatDefinitions::trickeryDescription(),
                    .value = stats.skills.trickery},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::stealthLabel(),
                    .helpDescription = model::CharacterStatDefinitions::stealthDescription(),
                    .value = stats.skills.stealth},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::socialLabel(),
                    .helpDescription = model::CharacterStatDefinitions::socialDescription(),
                    .value = stats.skills.social},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::magicItemUseLabel(),
                    .helpDescription = model::CharacterStatDefinitions::magicItemUseDescription(),
                    .value = stats.skills.magicItemUse},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::cookingLabel(),
                    .helpDescription = model::CharacterStatDefinitions::cookingDescription(),
                    .value = stats.skills.cooking},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::acrobaticsLabel(),
                    .helpDescription = model::CharacterStatDefinitions::acrobaticsDescription(),
                    .value = stats.skills.acrobatics},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::survivalLabel(),
                    .helpDescription = model::CharacterStatDefinitions::survivalDescription(),
                    .value = stats.skills.survival},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::focusLabel(),
                    .helpDescription = model::CharacterStatDefinitions::focusDescription(),
                    .value = stats.skills.focus},
                PageCharacterStatRowEntry{
                    .label = model::CharacterStatDefinitions::conditioningLabel(),
                    .helpDescription = model::CharacterStatDefinitions::conditioningDescription(),
                    .value = stats.skills.conditioning},
            },
        // clang-format on
        .width = scrollableContentW,
        .y = yAgg,
        .showModButtons = true,
        .buttonMinusDisabled = false,
        .buttonPlusDisabled = false,
    });
    scrollableStatsSection->addChild(section);
    yAgg += section->getDims().second;
  }

  if (useSingleColumn) {
    addDerivedStatSections(
        scrollableStatsSection, scrollableContentW, yAgg, derivedStats);
    scrollableStatsSection->build();
  } else {
    scrollableStatsSection->build();

    auto* scrollableDerivedSection = new SectionScrollable(window, modal);
    scrollableDerivedSection->setId("scrollableDerivedSection");
    scrollableDerivedSection->setPos(contentX + scrollableSectionWidth, contentY);
    scrollableDerivedSection->setScale(style.scale);
    scrollableDerivedSection->setProps(SectionScrollableProps{
        .width = scrollableSectionWidth,
        .height = unscaledContentH,
        .scrollBarWidth = 32,
        .bgColor = Colors::White,
    });
    addChild(scrollableDerivedSection);

    auto [scrollableDerivedContentW, scrollableDerivedContentH] =
        scrollableDerivedSection->getContentDims();

    int derivedYAgg = 0;
    addDerivedStatSections(
        scrollableDerivedSection, scrollableDerivedContentW, derivedYAgg, derivedStats);
    scrollableDerivedSection->build();
  }
}

void PageCharacter::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverShowLayerPopupText::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverShowLayerPopupText::onClick" << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || helpText.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::showLayerPopupText(window, title, helpText),
        0);
  }

} // namespace ui


namespace ui {

PageInventory::PageInventory(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageInventory::setProps(const PageInventoryProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageInventoryProps& PageInventory::getProps() { return props; }

const PageInventoryProps& PageInventory::getProps() const { return props; }

void PageInventory::populateInventoryProps(
    bmin::DynArray<ListInventoryPropsItem>& listProps) {
  if (!getStateManager() || !getDatabase()) {
    return;
  }
  auto& database = *getDatabase();

  model::CharacterPlayer equippedCheck;
  equippedCheck.equipment = props.equipment;

  for (const auto& item : props.inventory) {
    auto& itemTemplate = database.getItemTemplate(bmin::toStringView(item.itemName));
    const auto equippedSlot =
        model::characterPlayerGetEquipmentSlotForItemId(equippedCheck, item.id);
    bmin::String equippedSlotAbbrev;
    if (equippedSlot.has_value()) {
      equippedSlotAbbrev = model::characterEquipmentSlotAbbrev(*equippedSlot);
    }
    listProps.pushBack({.itemId = item.id,
                         .itemName = item.itemName,
                         .itemLabel = itemTemplate.label.empty() ? itemTemplate.name
                                                                 : itemTemplate.label,
                         .itemSprite = itemTemplate.iconSpriteName,
                         .isEquippable = model::itemTypeIsEquippable(itemTemplate.itemType),
                         .isEquipped = equippedSlot.has_value(),
                         .equippedSlotAbbrev = equippedSlotAbbrev,
                         .isStackable = itemTemplate.stackable,
                         .quantity = item.quantity});
  }
}

void PageInventory::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  ModalStandardProps modalProps;
  modalProps.width = style.width;
  modalProps.height = style.height;
  modalProps.portraitScale = props.portraitScale;
  if (!props.characterPlayerSprite.empty()) {
    modalProps.iconSprite = props.characterPlayerSprite;
  }
  modal->setProps(modalProps);
  syncHostStyleToCappedCentered(style);
  addChild(modal);

  if (!props.characterPlayerSprite.empty()) {
    if (auto* icon = modal->getChildById("headerIcon")) {
      auto [iconX, iconY] = icon->getPos();
      auto [iconW, iconH] = icon->getDims();
      auto iconBg = bmin::makeUnique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setPos(iconX, iconY);
      iconBg->setScale(1.f);
      iconBg->setProps(QuadProps{
          .width = iconW,
          .height = iconH,
          .bgColor = {255, 255, 255, 50},
      });

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(modalChildren.begin(),
                                             modalChildren.end(),
                                             [](const bmin::UniquePtr<UiElement>& child) {
                                               return child->getId() == "headerIcon";
                                             });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
      }
    }
  }

  auto closeButton = modal->getCloseButtonElement();
  if (closeButton) {
    closeButton->addEventObserver(
        new ObserverRemoveLayer(state::LayerId::Inventory));
  }

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = contentW / style.scale;
  int unscaledContentH = contentH / style.scale;

  // Create title element
  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text =
      bmin::String(TRANSLATE("Inventory")) + " - " + props.characterPlayerLabel;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto [subtitleX, subtitleY] = modal->getSubTitleLocation();

  const int partyIconSize = ButtonClose::closeButtonSize;
  const int scaledPartyIconSize = static_cast<int>(partyIconSize * style.scale);
  const int partyIconY = subtitleY - scaledPartyIconSize / 2;

  if (!props.partyMembers.empty()) {
    PartyMemberIconSelectorProps selectorProps;
    selectorProps.selectedIndex = props.partyMemberInventoryIndex;
    for (const auto& member : props.partyMembers) {
      selectorProps.members.pushBack(member.spriteName);
    }

    auto partySelector = new PartyMemberIconSelector(window, modal);
    partySelector->setId("partyMemberSelector");
    partySelector->setPos(subtitleX, partyIconY);
    partySelector->setScale(style.scale);
    partySelector->setProps(selectorProps);
    modal->addChild(partySelector);
  }

  const int statsRowHeight = 32;
  const int scaledStatsRowHeight = static_cast<int>(statsRowHeight * style.scale);
  const int statsRowPadding = static_cast<int>(16 * style.scale);
  const int statsRowY = contentY;
  const int scrollableY = contentY + scaledStatsRowHeight;
  const int scrollableHeight = unscaledContentH - statsRowHeight;

  auto statsBar = new Quad(window, modal);
  statsBar->setId("statsBar");
  statsBar->setPos(contentX, statsRowY);
  statsBar->setScale(style.scale);
  statsBar->setProps(QuadProps{
      .width = unscaledContentW,
      .height = statsRowHeight,
      .bgColor = Colors::White,
  });
  modal->addChild(statsBar);

  auto weightText = new TextLine(window, modal);
  weightText->setId("statsWeight");
  TextFontProps weightFont;
  setBaseFontConfig(weightFont, BaseFontConfig::MODAL_TEXT);
  TextLineProps weightProps;
  weightProps.fontFamily = weightFont.fontFamily;
  weightProps.fontSize = weightFont.fontSize;
  weightProps.fontColor = Colors::DarkGrey;
  weightProps.textAlign = TextAlign::LEFT_CENTER;
  weightProps.textBlocks.pushBack({
      .text = bmin::String(TRANSLATE("Carrying")) + " " + bmin::toString(props.weightCarrying) + "/" +
              bmin::toString(props.weightCapacity),
  });
  weightText->setScale(1.f);
  weightText->setProps(weightProps);
  weightText->setPos(contentX + statsRowPadding, statsRowY + scaledStatsRowHeight / 2);
  modal->addChild(weightText);

  auto goldText = new TextLine(window, modal);
  goldText->setId("statsGold");
  TextFontProps goldFont;
  setBaseFontConfig(goldFont, BaseFontConfig::MODAL_TEXT);
  TextLineProps goldProps;
  goldProps.fontFamily = goldFont.fontFamily;
  goldProps.fontSize = goldFont.fontSize;
  goldProps.fontColor = Colors::DarkGrey;
  goldProps.textAlign = TextAlign::LEFT_CENTER;
  goldProps.textBlocks.pushBack({
      .text = bmin::toString(props.gold) + bmin::String(TRANSLATE(" gp")),
      .fontColor = Colors::Blue,
  });
  goldText->setScale(1.f);
  goldText->setProps(goldProps);
  goldText->setPos(contentX + contentW - goldText->getDims().first - statsRowPadding,
                   statsRowY + scaledStatsRowHeight / 2);
  modal->addChild(goldText);

  // Create SectionScrollable for content area
  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  scrollableSection->setPos(contentX, scrollableY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = scrollableHeight,
      .scrollBarWidth = 40,
  });
  addChild(scrollableSection);

  // Create ListInventory inside the scrollable section
  auto listInventory = new ListInventory(window, scrollableSection);
  listInventory->setId("listInventory");
  listInventory->setPos(0, 0);
  listInventory->setScale(style.scale);

  ListInventoryProps listProps;
  listProps.characterPlayerId = props.characterPlayerId;
  listProps.width = static_cast<float>(contentW) / style.scale -
                    scrollableSection->getProps().scrollBarWidth - 8;
  populateInventoryProps(listProps.items);
  listInventory->setProps(listProps);
  scrollableSection->addChild(listInventory);
  scrollableSection->build();
}

void PageInventory::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

PageMagicSetup::PageMagicSetup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageMagicSetup::setProps(const PageMagicSetupProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageMagicSetupProps& PageMagicSetup::getProps() { return props; }

const PageMagicSetupProps& PageMagicSetup::getProps() const { return props; }

const std::pair<int, int> PageMagicSetup::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageMagicSetup::addPortraitBackground(ModalStandard* modal) {
  if (props.characterPlayerSprite.empty()) {
    return;
  }
  auto* icon = modal->getChildById("headerIcon");
  if (!icon) {
    return;
  }

  auto [iconX, iconY] = icon->getPos();
  auto [iconW, iconH] = icon->getDims();
  auto iconBg = bmin::makeUnique<Quad>(window, modal);
  iconBg->setId("headerIconBg");
  iconBg->setPos(iconX, iconY);
  iconBg->setScale(1.f);
  iconBg->setProps(QuadProps{
      .width = iconW,
      .height = iconH,
      .bgColor = {255, 255, 255, 50},
  });

  auto& modalChildren = modal->getChildren();
  const auto insertBefore =
      std::find_if(modalChildren.begin(),
                   modalChildren.end(),
                   [](const bmin::UniquePtr<UiElement>& child) {
                     return child->getId() == "headerIcon";
                   });
  if (insertBefore != modalChildren.end()) {
    modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
  }
}

void PageMagicSetup::addPartyMemberSelector(ModalStandard* modal) {
  if (props.partyMembers.empty()) {
    return;
  }

  auto [subtitleX, subtitleY] = modal->getSubTitleLocation();
  const int partyIconSize = ButtonClose::closeButtonSize;
  const int scaledPartyIconSize = static_cast<int>(partyIconSize * style.scale);
  const int partyIconY = subtitleY - scaledPartyIconSize / 2;

  PartyMemberIconSelectorProps selectorProps;
  selectorProps.selectedIndex = props.partyMemberMagicIndex;
  selectorProps.target = PartyMemberIconSelectorTarget::MAGIC;
  for (const auto& member : props.partyMembers) {
    selectorProps.members.pushBack(member.spriteName);
  }

  auto partySelector = new PartyMemberIconSelector(window, modal);
  partySelector->setId("partyMemberSelector");
  partySelector->setPos(subtitleX, partyIconY);
  partySelector->setScale(style.scale);
  partySelector->setProps(selectorProps);
  modal->addChild(partySelector);
}

bool PageMagicSetup::isNarrowLayout() const {
  if (props.width <= 0 || props.height <= 0) {
    return false;
  }
  return static_cast<float>(props.height) / static_cast<float>(props.width) >=
         narrowAspectMin;
}

int PageMagicSetup::equippedRowHeight() const {
  return std::max(manaSlotSize, editRunesButtonHeight) + statusStripPadding;
}

int PageMagicSetup::statusAreaHeight(bool narrow) const {
  if (narrow) {
    return equippedRowHeight() + statusStripPadding + availableRunesGridHeight;
  }
  return availableRunesGridHeight;
}

void PageMagicSetup::addRuneSlotRow(ModalStandard* modal,
                                   int contentX,
                                   int contentW,
                                   int rowY,
                                   int rowHeight,
                                   StatusAlign align) {
  const int scaledSlotSize = static_cast<int>(manaSlotSize * style.scale);
  const int scaledSlotGap = static_cast<int>(manaSlotGap * style.scale);
  const int scaledRowHeight = static_cast<int>(rowHeight * style.scale);
  const int scaledPadding = static_cast<int>(statusStripPadding * style.scale);
  const int scaledButtonW = static_cast<int>(editRunesButtonWidth * style.scale);
  const int scaledButtonH = static_cast<int>(editRunesButtonHeight * style.scale);
  const int scaledButtonGap = static_cast<int>(editRunesButtonGap * style.scale);
  const int slotY = rowY + (scaledRowHeight - scaledSlotSize) / 2;
  const int buttonY = rowY + (scaledRowHeight - scaledButtonH) / 2;
  const int scaledIconSize =
      static_cast<int>(manaSlotIconSize * manaSlotIconScale * style.scale);
  const int slotCount = static_cast<int>(props.runeSlots.size());
  const int slotsWidth =
      slotCount > 0 ? slotCount * scaledSlotSize + (slotCount - 1) * scaledSlotGap
                    : 0;
  const int rowContentWidth = scaledButtonW + scaledButtonGap + slotsWidth;
  int buttonX = contentX + scaledPadding;
  if (align == StatusAlign::Center) {
    buttonX = contentX + (contentW - rowContentWidth) / 2;
  } else if (align == StatusAlign::Right) {
    buttonX = contentX + contentW - scaledPadding - rowContentWidth;
  }
  const int stripX = buttonX + scaledButtonW + scaledButtonGap;

  auto editButton = new ButtonModal(window, modal);
  editButton->setId("editRunesButton");
  editButton->setPos(buttonX, buttonY);
  editButton->setScale(1.f);
  editButton->setProps(ButtonModalProps{
      .text = TRANSLATE("Runes"),
      .width = scaledButtonW,
      .height = scaledButtonH,
      .fontSize = sdl2w::TEXT_SIZE_14,
  });
  if (!props.characterPlayerId.empty()) {
    editButton->addEventObserver(
        new ObserverShowLayerEquipRunes(window, props.characterPlayerId));
  }
  modal->addChild(editButton);

  for (size_t i = 0; i < props.runeSlots.size(); ++i) {
    const auto& slot = props.runeSlots[i];
    const int slotX = stripX + static_cast<int>(i) * (scaledSlotSize + scaledSlotGap);
    const bool isSelected = props.selectedRuneSlotIndex == static_cast<int>(i);

    auto slotQuad = new Quad(window, modal);
    slotQuad->setId("runeSlot_" + bmin::toString(static_cast<int>(i)));
    slotQuad->setPos(slotX, slotY);
    slotQuad->setScale(1.f);
    slotQuad->setProps(QuadProps{
        .width = scaledSlotSize,
        .height = scaledSlotSize,
        .bgColor = slot.filled ? Colors::Grey2 : Colors::DarkGrey,
        .borderColor = isSelected ? Colors::ButtonModalSelected : Colors::Transparent,
        .borderSize = isSelected ? 2 : 0,
    });
    modal->addChild(slotQuad);

    if (slot.filled && !slot.iconSprite.empty()) {
      auto icon = new SpriteElement(window, slotQuad);
      icon->setId("runeSlotIcon");
      icon->setPos((scaledSlotSize - scaledIconSize) / 2,
                   (scaledSlotSize - scaledIconSize) / 2);
      icon->setScale(manaSlotIconScale * style.scale);
      icon->setProps(SpriteElementProps{
          .width = manaSlotIconSize,
          .height = manaSlotIconSize,
          .spriteName = slot.iconSprite,
      });
      slotQuad->addChild(icon);
    }
  }
}

void PageMagicSetup::addElementCountGrid(ModalStandard* modal,
                                         int contentX,
                                         int contentW,
                                         int gridTopY,
                                         StatusAlign align) {
  if (props.elementCounts.empty()) {
    return;
  }

  const int scaledCellW = static_cast<int>(elementCellWidth * style.scale);
  const int scaledCellH = static_cast<int>(elementCellHeight * style.scale);
  const int scaledPadding = static_cast<int>(statusStripPadding * style.scale);
  const int scaledIconSize =
      static_cast<int>(elementIconSize * elementIconScale * style.scale);
  const int gridWidth = elementGridCols * scaledCellW;
  int gridOriginX = contentX + scaledPadding;
  if (align == StatusAlign::Center) {
    gridOriginX = contentX + (contentW - gridWidth) / 2;
  } else if (align == StatusAlign::Right) {
    gridOriginX = contentX + contentW - scaledPadding - gridWidth;
  }

  TextFontProps countFont;
  setBaseFontConfig(countFont, BaseFontConfig::MODAL_TEXT);

  for (size_t i = 0; i < props.elementCounts.size(); ++i) {
    const auto& entry = props.elementCounts[i];
    const int col = static_cast<int>(i % elementGridCols);
    const int row = static_cast<int>(i / elementGridCols);
    const int cellX = gridOriginX + col * scaledCellW;
    const int cellY = gridTopY + row * scaledCellH;

    auto countText = new TextLine(window, modal);
    countText->setId("elementCount_" + bmin::toString(static_cast<int>(i)));
    countText->setScale(1.f);
    TextLineProps countProps;
    countProps.fontFamily = countFont.fontFamily;
    countProps.fontSize = sdl2w::TEXT_SIZE_14;
    countProps.fontColor = Colors::DarkGrey;
    countProps.textAlign = TextAlign::LEFT_TOP;
    countProps.textBlocks.pushBack({.text = bmin::toString(entry.count)});
    countText->setProps(countProps);
    auto [countW, countH] = countText->getDims();
    countText->setPos(cellX + (scaledCellW - countW) / 2, cellY);
    modal->addChild(countText);

    if (!entry.iconSprite.empty()) {
      auto icon = new SpriteElement(window, modal);
      icon->setId("elementIcon_" + bmin::toString(static_cast<int>(i)));
      icon->setPos(cellX + (scaledCellW - scaledIconSize) / 2, cellY + countH);
      icon->setScale(elementIconScale * style.scale);
      icon->setProps(SpriteElementProps{
          .width = elementIconSize,
          .height = elementIconSize,
          .spriteName = entry.iconSprite,
      });
      modal->addChild(icon);
    }
  }
}

ListMagicSpellsProps PageMagicSetup::makeSpellListProps(int width) const {
  ListMagicSpellsProps listProps;
  listProps.width = width;
  for (const auto& spell : props.spells) {
    bmin::String label = spell.label;
    label += " [";
    label += bmin::toString(spell.manaCost);
    label += "]";
    if (spell.ready) {
      label += " (r)";
    }
    listProps.spells.pushBack(ListMagicSpellsPropsSpell{
        .id = spell.id,
        .label = label,
        .iconSprite = spell.iconSprite,
        .requiredRuneSprites = spell.requiredRuneSprites,
    });
  }
  return listProps;
}

void PageMagicSetup::addSpellsPanel(int x, int y, int width, int height) {
  auto scrollable = new SectionScrollable(window, this);
  scrollable->setId("spellsSection");
  scrollable->setPos(x, y);
  scrollable->setScale(style.scale);
  scrollable->setProps(SectionScrollableProps{
      .width = width,
      .height = height,
      .scrollBarWidth = spellPanelScrollBarWidth,
      .bgColor = Colors::White,
  });
  addChild(scrollable);

  const int scaledPadding = static_cast<int>(spellPanelHeaderPadding * style.scale);
  const int listWidth = width - spellPanelScrollBarWidth - spellPanelHeaderPadding;

  auto header = new TextLine(window, scrollable);
  header->setId("spellsSection_header");
  header->setPos(scaledPadding, scaledPadding);
  header->setScale(1.f);
  TextFontProps headerFont;
  setBaseFontConfig(headerFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps headerProps;
  headerProps.fontFamily = headerFont.fontFamily;
  headerProps.fontSize = sdl2w::TEXT_SIZE_24;
  headerProps.fontColor = Colors::DarkBlue;
  headerProps.textAlign = TextAlign::LEFT_TOP;
  headerProps.textBlocks.pushBack({.text = TRANSLATE("Spells")});
  header->setProps(headerProps);
  scrollable->addChild(header);

  auto [headerW, headerH] = header->getDims();
  const int listY = scaledPadding + headerH;

  auto list = new ListMagicSpells(window, scrollable);
  list->setId("spellsSection_list");
  list->setPos(0, listY);
  list->setScale(style.scale);
  list->setProps(makeSpellListProps(listWidth));
  scrollable->addChild(list);

  scrollable->build();
}

void PageMagicSetup::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  ModalStandardProps modalProps;
  modalProps.width = style.width;
  modalProps.height = style.height;
  modalProps.portraitScale = props.portraitScale;
  if (!props.characterPlayerSprite.empty()) {
    modalProps.iconSprite = props.characterPlayerSprite;
  }
  modal->setProps(modalProps);
  syncHostStyleToCappedCentered(style);
  addChild(modal);

  addPortraitBackground(modal);

  auto closeButton = modal->getCloseButtonElement();
  if (closeButton) {
    closeButton->addEventObserver(
        new ObserverRemoveLayer(state::LayerId::Magic));
  }

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  const int unscaledContentW = static_cast<int>(contentW / style.scale);
  const int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  if (props.characterPlayerLabel.empty()) {
    titleBlock.text = TRANSLATE("Magic");
  } else {
    titleBlock.text =
        bmin::String(TRANSLATE("Magic")) + " - " + props.characterPlayerLabel;
  }
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  addPartyMemberSelector(modal);

  // Portrait / narrow (e.g. 400x900): stack equipped runes above available runes.
  // Landscape: keep them side-by-side in one status strip.
  const bool narrow = isNarrowLayout();
  const int statusHeight = statusAreaHeight(narrow);
  const int scaledStatusHeight = static_cast<int>(statusHeight * style.scale);
  const int scrollableY = contentY + scaledStatusHeight;
  const int scrollableHeight = unscaledContentH - statusHeight;

  if (narrow) {
    const int equippedHeight = equippedRowHeight();
    const int scaledEquippedHeight =
        static_cast<int>(equippedHeight * style.scale);
    const int scaledGap = static_cast<int>(statusStripPadding * style.scale);
    const int gridTopY = contentY + scaledEquippedHeight + scaledGap;
    addRuneSlotRow(modal,
                   contentX,
                   contentW,
                   contentY,
                   equippedHeight,
                   StatusAlign::Center);
    addElementCountGrid(
        modal, contentX, contentW, gridTopY, StatusAlign::Center);
  } else {
    addRuneSlotRow(modal,
                   contentX,
                   contentW,
                   contentY,
                   availableRunesGridHeight,
                   StatusAlign::Left);
    addElementCountGrid(
        modal, contentX, contentW, contentY, StatusAlign::Right);
  }

  addSpellsPanel(contentX, scrollableY, unscaledContentW, scrollableHeight);
}

void PageMagicSetup::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverShowLayerEquipRunes::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    LOG(INFO) << "ObserverShowLayerEquipRunes::onClick character="
              << characterPlayerId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || characterPlayerId.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::showLayerEquipRunes(window, characterPlayerId),
        0);
  }

void ObserverSetSpellReady::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    LOG(INFO) << "ObserverSetSpellReady::onClick spell=" << spellName
              << " ready=" << ready << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::setSpellReady(characterPlayerId, spellName, ready),
        0);
  }

void ObserverToggleManaSlotRune::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    LOG(INFO) << "ObserverToggleManaSlotRune::onClick slot=" << slotIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::toggleManaSlotRune(characterPlayerId, slotIndex),
        0);
  }

} // namespace ui


namespace ui {

bmin::DynArray<TextBlock>
PageTalkChoice::colorizeDialogueByQuotes(const bmin::DynArray<TextBlock>& blocks,
                                         SDL_Color outsideColor) {
  bmin::DynArray<TextBlock> result;
  for (const auto& source : blocks) {
    const auto& text = source.text;
    if (text.empty()) {
      continue;
    }

    bool inQuotes = false;
    size_t segmentStart = 0;
    auto emitSegment = [&](size_t end, bool quoted) {
      if (end <= segmentStart) {
        return;
      }
      TextBlock piece;
      piece.text = text.substr(segmentStart, end - segmentStart);
      piece.fontFamily = source.fontFamily;
      piece.fontSize = source.fontSize;
      if (source.fontColor.has_value()) {
        // Preserve caller-set colors (e.g. echoed player choices in history).
        piece.fontColor = source.fontColor;
      } else if (quoted) {
        piece.fontColor = Colors::Charcoal;
      } else {
        piece.fontColor = outsideColor;
      }
      result.pushBack(piece);
    };

    for (size_t i = 0; i < text.size(); i++) {
      if (text[i] != '"') {
        continue;
      }
      if (inQuotes) {
        emitSegment(i + 1, true);
        segmentStart = i + 1;
        inQuotes = false;
      } else {
        emitSegment(i, false);
        segmentStart = i;
        inQuotes = true;
      }
    }
    emitSegment(text.size(), inQuotes);
  }
  return result;
}

PageTalkChoice::PageTalkChoice(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageTalkChoice::setProps(const PageTalkChoiceProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageTalkChoiceProps& PageTalkChoice::getProps() { return props; }

const PageTalkChoiceProps& PageTalkChoice::getProps() const { return props; }

const std::pair<int, int> PageTalkChoice::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageTalkChoice::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  // Create ModalStandard layout
  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  // FullBleed: LayerSpecialEvent talk path passes window dims and expects the dialogue
  // shell to fill the window (history + choice panes). CappedCentered would shrink the
  // talk UI on landscape and fight that layout.
  modal->setProps(ModalStandardProps{
      .width = style.width,
      .height = style.height,
      .layoutFit = LayoutFit::FullBleed,
      .iconSprite = props.portraitSpriteName,
      .portraitScale = props.portraitScale,
  });

  children.pushBack(bmin::UniquePtr<UiElement>(modal));

  if (!props.portraitSpriteName.empty()) {
    if (auto* border =
            dynamic_cast<BorderModalStandard*>(modal->getChildById("border"))) {
      auto [iconX, iconY] = border->getIconBorderLocation();
      const int iconSize = border->getProps().iconSize;
      auto iconBg = bmin::makeUnique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setPos(iconX, iconY);
      iconBg->setScale(style.scale);
      iconBg->setProps(QuadProps{
          .width = iconSize,
          .height = iconSize,
          .bgColor = Colors::OffWhite,
      });

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(modalChildren.begin(),
                                             modalChildren.end(),
                                             [](const bmin::UniquePtr<UiElement>& child) {
                                               return child->getId() == "headerIcon";
                                             });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
      } else {
        modal->addChild(iconBg.release());
      }
    }
  }

  auto [scaledContentW, scaledContentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();

  auto choiceSectionHeight = props.choiceAreaHeight;
  auto textSectionHeight =
      (scaledContentH / style.scale - BorderModalStandard::BOTTOM_BORDER_HEIGHT -
       choiceSectionHeight);
  auto borderHeight = BorderModalStandard::BOTTOM_BORDER_HEIGHT;
  auto scrollBarWidth = 32;

  // Create title element
  auto title = new TextLine(window, this);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = sdl2w::TEXT_SIZE_24;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = props.title;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  // Create SectionScrollable for content area
  auto textSection = new SectionScrollable(window, this);
  textSection->setId("textSection");
  textSection->setPos(contentX, contentY);
  textSection->setScale(style.scale);
  textSection->setProps(SectionScrollableProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = static_cast<int>(textSectionHeight),
      .scrollBarWidth = scrollBarWidth,
  });
  addChild(textSection);
  auto [textScrollableContentWidthScaled, textViewportHeightScaled] =
      textSection->getContentDims();

  TextFontProps textFont;
  setBaseFontConfig(textFont, BaseFontConfig::MODAL_CHOICE_TEXT);

  const int pinFrom =
      std::clamp(props.pinFromBlockIndex, 0, static_cast<int>(props.textBlocks.size()));
  bmin::DynArray<TextBlock> historyBlocksRaw;
  bmin::DynArray<TextBlock> currentBlocksRaw;
  for (int i = 0; i < static_cast<int>(props.textBlocks.size()); i++) {
    if (i < pinFrom) {
      historyBlocksRaw.pushBack(props.textBlocks[i]);
    } else {
      currentBlocksRaw.pushBack(props.textBlocks[i]);
    }
  }
  const auto historyBlocks = colorizeDialogueByQuotes(historyBlocksRaw, Colors::Grey2);
  const auto currentBlocks = colorizeDialogueByQuotes(currentBlocksRaw, Colors::Grey2);

  int contentYOffset = 0;
  int historyHeightScaled = 0;

  auto addParagraph = [&](const bmin::DynArray<TextBlock>& blocks,
                          const bmin::String& id) -> TextParagraph* {
    if (blocks.empty()) {
      return nullptr;
    }
    auto* paragraph = new TextParagraph(window, textSection);
    paragraph->setId(id);
    paragraph->setPos(0, contentYOffset);
    paragraph->setScale(1.f);
    paragraph->setProps(TextParagraphProps{
        .textBlocks = blocks,
        .width = textScrollableContentWidthScaled,
        .bgColor = Colors::OffWhite,
        .padding = 4,
        .lineSpacing = 0,
        .lineHeightScale = props.lineHeightScale,
        .blankLineHeightScale = props.blankLineHeightScale,
        .fontFamily = textFont.fontFamily,
        .fontSize = textFont.fontSize,
        .fontColor = textFont.fontColor,
    });
    textSection->addChild(paragraph);
    const int height = paragraph->getDims().second;
    contentYOffset += height;
    return paragraph;
  };

  if (auto* historyParagraph = addParagraph(historyBlocks, "textBlocksHistory")) {
    historyHeightScaled = historyParagraph->getDims().second;
  }

  int currentHeightScaled = 0;
  if (auto* currentParagraph = addParagraph(currentBlocks, "textBlocks")) {
    currentHeightScaled = currentParagraph->getDims().second;
  }

  // Pad so the pinned (current) dialogue can sit at the top of the viewport.
  if (currentHeightScaled > 0) {
    const int padHeightScaled =
        std::max(0, textViewportHeightScaled - currentHeightScaled);
    if (padHeightScaled > 0) {
      auto* spacer = new Quad(window, textSection);
      spacer->setId("textBottomPad");
      spacer->setPos(0, contentYOffset);
      spacer->setScale(1.f);
      spacer->setProps(QuadProps{
          .width = textScrollableContentWidthScaled,
          .height = padHeightScaled,
          .bgColor = Colors::OffWhite,
      });
      textSection->addChild(spacer);
    }
  }

  textSection->build();
  textSection->scrollTo(historyHeightScaled);

  auto sepBorder = new OutsetRectangle(window, this);
  sepBorder->setPos(contentX, contentY + textSectionHeight * style.scale);
  sepBorder->setScale(style.scale);
  sepBorder->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = 10,
  });
  addChild(sepBorder);

  auto choiceSection = new SectionScrollable(window, this);
  choiceSection->setId("choiceSection");
  choiceSection->setPos(contentX,
                        contentY + (textSectionHeight + borderHeight) * style.scale);
  choiceSection->setScale(style.scale);
  choiceSection->setProps(SectionScrollableProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = choiceSectionHeight,
      .scrollBarWidth = scrollBarWidth,
      .indicatorHeight = 0,
  });
  addChild(choiceSection);

  // Create choices (setPos before setProps so ButtonTextWrap builds text at the right
  // offset)
  auto choiceYOffset = 0;
  for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
    auto choiceButton = new ButtonTextWrap(window, choiceSection);
    choiceButton->setId("choice" + bmin::toString(i));
    TextFontProps choiceFont;
    setBaseFontConfig(choiceFont, BaseFontConfig::MODAL_CHOICE_TEXT);
    ui::ButtonTextWrapProps choiceButtonProps;
    const bmin::String& prefixText = props.choices[i].prefixText;
    const bmin::String choiceText =
        " " + bmin::toString(i + 1) + ". " +
        ((prefixText.empty() ? props.choices[i].text
                             : prefixText + " " + props.choices[i].text));
    const SDL_Color choiceColor =
        props.choices[i].previouslyChosen ? Colors::Grey : Colors::DarkBlue;
    choiceButtonProps.isSelected = false;
    choiceButtonProps.textParagraph.textBlocks.pushBack(
        TextBlock{.text = choiceText, .fontColor = choiceColor});
    choiceButtonProps.textParagraph.width =
        scaledContentW - 8 * style.scale - scrollBarWidth * style.scale;
    choiceButtonProps.textParagraph.fontFamily = choiceFont.fontFamily;
    choiceButtonProps.textParagraph.fontSize = choiceFont.fontSize;
    choiceButtonProps.textParagraph.fontColor = choiceColor;
    choiceButtonProps.textParagraph.lineHeightScale = 0.85f;
    choiceButtonProps.verticalPadding = 0;
    choiceButton->setScale(1.f);
    choiceButton->setPos(4 * style.scale, choiceYOffset);
    choiceButton->setProps(choiceButtonProps);
    auto [choiceWidth, choiceHeight] = choiceButton->getDims();
    choiceSection->addChild(choiceButton);
    choiceYOffset += choiceHeight;
  }

  choiceSection->build();
}

void PageTalkChoice::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

PageModalEvent::PageModalEvent(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageModalEvent::setProps(const PageModalEventProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageModalEventProps& PageModalEvent::getProps() { return props; }

const PageModalEventProps& PageModalEvent::getProps() const { return props; }

const std::pair<int, int> PageModalEvent::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageModalEvent::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
      .enableCloseButton = false,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);
  addChild(modal);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  titleProps.textBlocks.pushBack({.text = props.title});
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto* border = dynamic_cast<BorderModalSmall*>(modal->getChildById("border"));
  auto [contentX, contentY] = modal->getContentLocation();
  int contentW = 0;
  int contentH = 0;
  if (props.showContinueButton && props.choices.empty()) {
    // Reserve the bottom button strip for Continue.
    auto [w, h] = modal->getContentDims();
    contentW = w;
    contentH = h;
  } else if (border) {
    // No Continue row — use the full content area (choices live in the scroll body).
    auto [w, h] = border->getContentDims();
    contentW = w;
    contentH = h;
  }

  const int unscaledContentW = static_cast<int>(contentW / style.scale);
  const int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("textSection");
  scrollableSection->setPos(contentX, contentY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = unscaledContentH,
  });
  auto [scrollableContentW, scrollableViewportH] = scrollableSection->getContentDims();

  auto textBlock = new TextParagraph(window, scrollableSection);
  textBlock->setId("textBlocks");
  TextFontProps textFont;
  setBaseFontConfig(textFont, BaseFontConfig::MODAL_TEXT);
  textBlock->setPos(0, 0);
  textBlock->setScale(1.f);
  textBlock->setProps(TextParagraphProps{
      .textBlocks = props.textBlocks,
      .width = scrollableContentW,
      .bgColor = Colors::OffWhite,
      .padding = 4,
      .lineSpacing = 0,
      .fontFamily = textFont.fontFamily,
      .fontSize = textFont.fontSize,
      .fontColor = Colors::Black,
  });
  scrollableSection->addChild(textBlock);

  int contentBottom = textBlock->getDims().second;
  if (!props.choices.empty()) {
    int choiceYOffset = contentBottom;
    for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
      auto choiceButton = new ButtonTextWrap(window, scrollableSection);
      choiceButton->setId("choice" + bmin::toString(i));
      TextFontProps choiceFont;
      setBaseFontConfig(choiceFont, BaseFontConfig::MODAL_CHOICE_TEXT);
      ButtonTextWrapProps choiceButtonProps;
      const bmin::String& prefixText = props.choices[i].prefixText;
      const bmin::String choiceText =
          bmin::toString(i + 1) + ". " +
          (prefixText.empty() ? props.choices[i].text
                              : prefixText + " " + props.choices[i].text);
      const SDL_Color choiceColor =
          props.choices[i].previouslyChosen ? Colors::Grey : Colors::DarkBlue;
      choiceButtonProps.textParagraph.textBlocks.pushBack(
          TextBlock{.text = choiceText, .fontColor = choiceColor});
      choiceButtonProps.textParagraph.width = scrollableContentW - 8;
      choiceButtonProps.textParagraph.fontFamily = choiceFont.fontFamily;
      choiceButtonProps.textParagraph.fontSize = choiceFont.fontSize;
      choiceButtonProps.textParagraph.fontColor = choiceColor;
      choiceButton->setScale(1.f);
      choiceButton->setPos(4, choiceYOffset);
      choiceButton->setProps(choiceButtonProps);
      auto [__, choiceHeight] = choiceButton->getDims();
      choiceYOffset += choiceHeight;
      scrollableSection->addChild(choiceButton);
    }
    contentBottom = choiceYOffset;
  }

  // Fill remaining viewport so short text reaches the button strip (talk modal pattern).
  const int padHeight = std::max(0, scrollableViewportH - contentBottom);
  if (padHeight > 0) {
    auto* spacer = new Quad(window, scrollableSection);
    spacer->setId("textBottomPad");
    spacer->setPos(0, contentBottom);
    spacer->setScale(1.f);
    spacer->setProps(QuadProps{
        .width = scrollableContentW,
        .height = padHeight,
        .bgColor = Colors::OffWhite,
    });
    scrollableSection->addChild(spacer);
  }

  scrollableSection->build();
  modal->addChild(scrollableSection);

  if (props.showContinueButton && props.choices.empty()) {
    auto [buttonsW, buttonsH] = modal->getButtonsDims();
    auto [buttonsX, buttonsY] = modal->getButtonsLocation();
    const int buttonPadding = 2;
    const int buttonWidth = 120;

    auto buttonGroup = new ButtonGroup(window, modal);
    buttonGroup->setId("buttonGroup");
    buttonGroup->setPos(buttonsX, buttonsY);
    buttonGroup->setScale(style.scale);
    buttonGroup->setProps(ButtonGroupProps{
        .width = static_cast<int>(buttonsW / style.scale),
        .alignment = ButtonGroupAlignment::RIGHT,
        .buttonWidth = buttonWidth,
        .buttonHeight = ModalSmall::BUTTONS_AREA_HEIGHT - 2 * buttonPadding,
        .padding = buttonPadding,
        .buttons = {{.label = TRANSLATE("Okay"), .type = ButtonGroupButtonType::MODAL}},
    });
    modal->addChild(buttonGroup);
  }
}

void PageModalEvent::render(int dt) { UiElement::render(dt); }

} // namespace ui
