#include "PageCharacter.h"
#include "sdl2w/L10n.h"
#include "model/instances/CharacterPlayer.h"
#include "model/stats/CharacterDerivedStatDefinitions.h"
#include "model/stats/CharacterDerivedStats.h"
#include "model/stats/CharacterStatDefinitions.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/VerticalList.h"
#include "ui/elements/buttons/ButtonIcon.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/observers/ObserverShowLayerPopupText.hpp"
#include <utility>

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
