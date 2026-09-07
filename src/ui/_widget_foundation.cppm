module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <string_view>
#include <string>
#include <cmath>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <limits>

export module carcer.ui.widgets.foundation;
export import bmin.containers;
export import carcer.state;
export import carcer.ui.core;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

// --- from ui/components/ChCompactInfo.h ---
// IWYU pragma: keep

namespace ui {

struct ChCompactInfoProps {
  bmin::String characterSpriteName;
  bmin::DynArray<bmin::String> statusEffectSpriteNames;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int statusIconSize = 12;
  int numStatusColumns = 2;

  int hp = 0;
  int mana = 0;
  bool isSelected = false;
  int padding = 4;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_18;
};

// ChCompactInfo component - renders a character sprite, status effect icons,
// and health/mana values in a bordered box.
class ChCompactInfo : public UiElement {
private:
  ChCompactInfoProps props;

  int fontHeight = 20;

  int getContentWidth() const;
  int getContentHeight() const;

public:
  ChCompactInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ChCompactInfo() override = default;

  void setProps(const ChCompactInfoProps& _props);
  ChCompactInfoProps& getProps();
  const ChCompactInfoProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/HorizontalList.h ---
// IWYU pragma: keep

namespace ui {

struct HorizontalListProps {
  int height = 0;
  int lineWidth = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// HorizontalList element - lays out child UiElements in a row.
class HorizontalList : public UiElement {
private:
  HorizontalListProps props;
  int selectedIndex = -1;

public:
  HorizontalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~HorizontalList() override = default;

  void setProps(const HorizontalListProps& _props);
  HorizontalListProps& getProps();
  const HorizontalListProps& getProps() const;

  void setSelectedIndex(int index);
  int getSelectedIndex() const;
  void clearSelection();

  void addListItem(UiElement* item);
  void addListItems(const bmin::DynArray<UiElement*>& items);
  void removeListItemAtIndex(size_t index);

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/OutsetRectangle.h ---
namespace ui {

// OutsetRectangle-specific properties
struct OutsetRectangleProps {
  int width = 0;
  int height = 0;
  SDL_Color color = Colors::BorderModalStandard;
  SDL_Color colorTopRight = Colors::BorderModalStandardLight;
  SDL_Color colorBottomLeft = Colors::BorderModalStandardDark;
  int borderSize = 4;
};

// OutsetRectangle element - renders a rectangle with outset border effect
// Position/scale via setPos/setScale; size via props → build
class OutsetRectangle : public UiElement {
private:
  OutsetRectangleProps props;

public:
  OutsetRectangle(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~OutsetRectangle() override = default;

  // Setters and getters for OutsetRectangle-specific properties
  void setProps(const OutsetRectangleProps& _props);
  OutsetRectangleProps& getProps();
  const OutsetRectangleProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/Quad.h ---
// IWYU pragma: keep

#if defined(MIYOOA30) || defined(MIYOOMINI)

#else

#endif

namespace ui {

// Quad-specific properties
struct QuadProps {
  int width = 0;
  int height = 0;
  SDL_Color bgColor = SDL_Color{0, 0, 0, 0};
  bmin::String bgSprite;
  SDL_Color borderColor = SDL_Color{0, 0, 0, 0};
  int borderSize = 0;
};

// Quad element - renders a stylized rectangle with children
// Position/scale via setPos/setScale; size via props.width/height → build
class Quad : public UiElement {
private:
  SDL_Texture* renderTexture = nullptr;
  int currentWidth = 0;
  int currentHeight = 0;

  QuadProps props;

  void createRenderTexture();
  void destroyRenderTexture();

public:
  Quad(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~Quad() override;

  // Setters and getters for quad-specific properties
  void setProps(const QuadProps& _props);
  QuadProps& getProps();
  const QuadProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseWheelEvent(int mouseX,
                            int mouseY,
                            int delta,
                            bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/SpriteElement.h ---
namespace ui {

struct SpriteElementProps {
  int width = 0;
  int height = 0;
  bmin::String spriteName;
};

// Sprite element - renders a stylized sprite
// Position/scale via setPos/setScale; size/sprite via props → build
class SpriteElement : public UiElement {
private:
  SpriteElementProps props;
  sdl2w::Sprite sprite;

public:
  SpriteElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SpriteElement() override = default;

  void setProps(const SpriteElementProps& _props);
  SpriteElementProps& getProps();
  const SpriteElementProps& getProps() const;

  // Convenience: sets sprite name and rebuilds
  void setSprite(const bmin::String& name);
  const sdl2w::Sprite& getSprite() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/TextLine.h ---
// IWYU pragma: keep

namespace ui {

// Individual text block with optional style overrides
struct TextBlock {
  bmin::String text;

  // Optional style overrides - if not set, uses TextLineProps defaults
  std::optional<FontFamily> fontFamily;
  std::optional<sdl2w::TextSize> fontSize;
  std::optional<SDL_Color> fontColor;
};

// TextLine-specific properties
struct TextLineProps {
  bmin::DynArray<TextBlock> textBlocks;
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
  TextAlign textAlign = TextAlign::LEFT_TOP;
};

struct TextLineRenderTextParams {
  bmin::String text;
  sdl2w::RenderTextParams params;
};

// TextLine element - renders a stylized line of text
class TextLine : public UiElement {
private:
  TextLineProps props;
  bmin::DynArray<bmin::UniquePtr<TextLineRenderTextParams>> textRenderables;

  sdl2w::RenderTextParams makeRenderTextParams(const TextBlock& block) const;

public:
  TextLine(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextLine() override = default;

  // Static utility method to convert FontFamily to font name
  static bmin::String getFontNameFromFamily(FontFamily fontFamily);

  void setProps(const TextLineProps& _props);
  TextLineProps& getProps();
  const TextLineProps& getProps() const;

  // Position is baked into renderables during build
  void setPos(int x, int y) override;
  void setScale(float scale) override;

  std::pair<int, int> calculateTextDims() const;
  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/VerticalList.h ---
// IWYU pragma: keep

namespace ui {

struct VerticalListProps {
  int width = 0;
  int lineHeight = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// VerticalList element - renders an opinionated list of UiElements
class VerticalList : public UiElement {
private:
  VerticalListProps props;
  int selectedIndex = -1;

public:
  VerticalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~VerticalList() override = default;

  void setProps(const VerticalListProps& _props);
  VerticalListProps& getProps();
  const VerticalListProps& getProps() const;

  void setSelectedIndex(int index);
  int getSelectedIndex() const;
  void clearSelection();

  void addListItem(UiElement* item);
  void addListItems(const bmin::DynArray<UiElement*>& items);
  void removeListItemAtIndex(size_t index);

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonClose.h ---
namespace ui {

// Close type enum for close buttons
enum class CloseType { MODAL, POPUP };

// ButtonClose-specific properties
struct ButtonCloseProps {
  CloseType closeType = CloseType::MODAL;
  int xLength = 13;
};

// ButtonClose element - renders a clickable button typically used to close a modal or
// popup window Uses Position, Size, Scale from BaseStyle
class ButtonClose : public UiElement {
private:
  ButtonCloseProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  constexpr static int closeButtonSize = 32;
  ButtonClose(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonClose() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonCloseProps& _props);
  ButtonCloseProps& getProps();
  const ButtonCloseProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonScroll.h ---
namespace ui {

// Direction enum for scroll buttons
enum class ScrollDirection {
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

// ButtonScroll-specific properties
struct ButtonScrollProps {
  ScrollDirection direction = ScrollDirection::UP;
  bool isSelected = false;
  bool isDisabled = false;
  int width = 32;
  int height = 32;
};

// ButtonScroll element - renders a square clickable button used to scroll windows up/down
// Uses Position, Size, Scale from BaseStyle
class ButtonScroll : public UiElement {
private:
  ButtonScrollProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonScroll(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonScroll() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonScrollProps& _props);
  ButtonScrollProps& getProps();
  const ButtonScrollProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonSprite.h ---
namespace ui {

struct ButtonSpriteProps {
  bmin::String spriteName;
  int spriteWidth = 16;
  int spriteHeight = 16;
  int padding = 2;
  bool isSelected = false;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;
  int borderSize = 2;

  SDL_Color selectedBgColor = Colors::ButtonModalSelected;
  SDL_Color selectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color selectedBgColorBottomLeft = Colors::ButtonModalSelected;
  int selectedBorderSize = 2;
};

// ButtonSprite - clickable button displaying a sprite with uniform padding.
class ButtonSprite : public UiElement {
private:
  ButtonSpriteProps props;
  bool isInActiveMode = false;

  int getLogicalWidth() const;
  int getLogicalHeight() const;

public:
  bool isActive = false;
  ButtonSprite(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonSprite() override = default;

  void setProps(const ButtonSpriteProps& _props);
  ButtonSpriteProps& getProps();
  const ButtonSpriteProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonIcon.h ---
namespace ui {

struct ButtonIconProps {
  bmin::String regularSprite;
  bmin::String activeSprite;
  int iconSize = 32;
  bool isDisabled = false;
};

// ButtonIcon - clickable button that displays a regular or active sprite.
class ButtonIcon : public UiElement {
private:
  ButtonIconProps props;
  bool isInActiveMode = false;

public:
  static inline constexpr const char* MINUS_ICON1 = "ui_icon_buttons_0";
  static inline constexpr const char* MINUS_ICON2 = "ui_icon_buttons_8";
  static inline constexpr const char* PLUS_ICON1 = "ui_icon_buttons_1";
  static inline constexpr const char* PLUS_ICON2 = "ui_icon_buttons_9";
  static inline constexpr const char* QUESTION_ICON1 = "ui_icon_buttons_2";
  static inline constexpr const char* QUESTION_ICON2 = "ui_icon_buttons_10";
  static inline constexpr const char* HAMBURGER_ICON1 = "ui_icon_buttons_3";
  static inline constexpr const char* HAMBURGER_ICON2 = "ui_icon_buttons_11";

  bool isActive = false;
  ButtonIcon(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonIcon() override = default;

  void setProps(const ButtonIconProps& _props);
  ButtonIconProps& getProps();
  const ButtonIconProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonMove.h ---
namespace ui {

enum class MoveDirection {
  UpLeft,
  Up,
  UpRight,
  Left,
  Wait,
  Right,
  DownLeft,
  Down,
  DownRight,
};

struct ButtonMoveProps {
  MoveDirection direction = MoveDirection::Up;
};

// ButtonMove - clickable sprite button for touch movement directions.
class ButtonMove : public UiElement {
  // move_buttons.png layout (256x88):
  // rows 0-1: 22x22 half/diagonal buttons (5 per row, 11 columns in the sprite grid)
  // rows 2-3: 44x22 cardinal buttons (4 per row, 5 columns in the sprite grid)
  static constexpr int halfSpriteWidth = 22;
  static constexpr int halfSpriteHeight = 22;
  static constexpr int cardinalSpriteWidth = 44;
  static constexpr int cardinalSpriteHeight = 22;
  static constexpr int halfUnpressedBase = 0;
  static constexpr int halfPressedRowOffset = 11;
  static constexpr int cardinalUnpressedBase = 10;
  static constexpr int cardinalPressedRowOffset = 5;

  ButtonMoveProps props;
  bool isInActiveMode = false;

  static bool isHalfDirection(MoveDirection direction);
  static int getSpriteIndex(MoveDirection direction);
  bmin::String getSpriteName(bool pressed) const;

public:
  bool isActive = false;
  ButtonMove(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonMove() override = default;

  void setProps(const ButtonMoveProps& _props);
  ButtonMoveProps& getProps();
  const ButtonMoveProps& getProps() const;

  void setPos(int x, int y) override;
  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonWorldAction.h ---
namespace ui {

// ButtonWorldAction-specific properties
struct ButtonWorldActionProps {
  state::WorldActionType worldActionType;
};

struct ButtonWorldActionMapping {
  bmin::String label;
  int spriteIndex;
  bool isSmall = false;
};

// ButtonWorldAction element - renders a clickable button typically used to interact with
// the world Uses Position, Size, Scale from BaseStyle
class ButtonWorldAction : public UiElement {
private:
  ButtonWorldActionProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;
  const bmin::String spriteSheetName = "ui_action_buttons";
  const int normalStartingSpriteIndex = 16;
  const int smallStartingSpriteIndex = 0;
  const int normalSpriteOffsetToActive = 16;
  const int smallSpriteOffsetToActive = 8;

  static ButtonWorldActionMapping
  getButtonWorldActionMapping(state::WorldActionType worldActionType);

public:
  bool isActive = false;
  bool isModeSelected = false;
  ButtonWorldAction(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonWorldAction() override = default;

  static bool checkIfWorldActionButtonIsSmall(state::WorldActionType worldActionType);

  void setProps(const ButtonWorldActionProps& _props);
  ButtonWorldActionProps& getProps();
  const ButtonWorldActionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonModal.h ---
namespace ui {

// ButtonModal-specific properties
struct ButtonModalProps {
  bmin::String text;
  bool isSelected = false;
  int width = 80;
  int height = 32;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;

  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_20;
  SDL_Color fontColor = Colors::White;
};

// ButtonModal element - renders a clickable button typically used inside modal windows
// Position/scale via setPos/setScale; size via props.width/height → build
class ButtonModal : public UiElement {
private:
  ButtonModalProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonModal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonModal() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonModalProps& _props);
  ButtonModalProps& getProps();
  const ButtonModalProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/TextBanner.h ---
namespace ui {

enum class TextBannerCorner { LEFT_TOP, LEFT_BOTTOM, RIGHT_TOP, RIGHT_BOTTOM };

struct TextBannerProps {
  std::pair<int, int> location = {0, 0};
  std::pair<int, int> dims = {0, 0};
  TextBannerCorner corner = TextBannerCorner::LEFT_TOP;
  bmin::String text;
  SDL_Color backgroundColor = Colors::Grey;
  int padding = 6;
  int outsetBorderSize = 0;
  FontFamily fontFamily = FontFamily::TEXT_BOLD;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::White;
};

// TextBanner element - text with an outset rectangle background, placed in a corner
// of a container defined by location and dims props.
class TextBanner : public UiElement {
private:
  TextBannerProps props;

  std::pair<int, int> measureTextScaled() const;
  std::pair<int, int> calculateBannerScreenPosition(int bannerScaledWidth,
                                                      int bannerScaledHeight) const;

public:
  TextBanner(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextBanner() override = default;

  void setProps(const TextBannerProps& _props);
  TextBannerProps& getProps();
  const TextBannerProps& getProps() const;

  void setPos(int x, int y) override;
  void setScale(float scale) override;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/TextParagraph.h ---
// IWYU pragma: keep

namespace ui {

// TextParagraph-specific properties
struct TextParagraphProps {
  bmin::DynArray<TextBlock> textBlocks;
  int width = 0;
  SDL_Color bgColor = Colors::Transparent;
  int padding = 0;
  int lineSpacing = 0;
  // Multiplier for spacing between lines (1 = measured font height). Glyphs still
  // paint at full size; container height keeps room for the last line's descenders.
  float lineHeightScale = 1.f;
  // Multiplier for blank lines created by consecutive newlines (`\n\n`).
  // Relative to measured font height: 1 = full line, 0.5 = half, 0 = no gap.
  float blankLineHeightScale = 1.f;
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
  TextAlign textAlign = TextAlign::LEFT_TOP;
};

struct TextParagraphGeneratedBlock {
  int lineNumber;
  TextBlock textBlock;
  bmin::String text;
  int textWidth;
  int textHeight;
};

// TextParagraph element - lays out wrapped text into TextLines rendered via an internal Quad
class TextParagraph : public UiElement {
private:
  TextParagraphProps props;
  bmin::DynArray<TextParagraphGeneratedBlock> generatedBlocks;
  bmin::UniquePtr<Quad> quad;

  int getContentHeight() const;

public:
  TextParagraph(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextParagraph() override = default;

  void setProps(const TextParagraphProps& _props);
  TextParagraphProps& getProps();
  const TextParagraphProps& getProps() const;
  size_t getNumLines() const;
  virtual const std::pair<int, int> getDims() const override;

  void setPos(int x, int y) override;
  void setScale(float scale) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonList.h ---
namespace ui {

struct ButtonListProps {
  bmin::String text;
  std::optional<ScrollDirection> arrow;
  bool isSelected = false;
  int width = 14;
  int height = 14;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;
  SDL_Color arrowColor = Colors::White;
  SDL_Color fontColor = Colors::White;
};

// ButtonList - square button for list rows (modal styling for now)
class ButtonList : public UiElement {
private:
  ButtonListProps props;
  bool isInActiveMode = false;

public:
  static constexpr int defaultLogicalSize = 14;

  bool isActive = false;
  ButtonList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonList() override = default;

  static int yForListRow(int rowHeight, int btnLogicalSize, float scale);

  void setProps(const ButtonListProps& _props);
  ButtonListProps& getProps();
  const ButtonListProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/HorizontalSlider.h ---
namespace ui {

struct HorizontalSliderProps {
  int minValue = 1;
  int maxValue = 1;
  int value = 1;
  int width = 220;
  int height = 56;
  int sliderBarHeight = 32;
  int indicatorWidth = 24;
  SDL_Color labelColor = Colors::Black;
};

// Boilerplate HorizontalSlider: basic API + static rendering.
class HorizontalSlider : public UiElement {
  HorizontalSliderProps props;
  bool isDraggingIndicator = false;
  void refreshValueUi();
  bool isInSliderTrack(int mouseX, int mouseY) const;
  bool hitIndicator(int mouseX, int mouseY);
  bool hitButton(int mouseX, int mouseY);
  void setValueFromIndicatorMouseX(int mouseX);

public:
  HorizontalSlider(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~HorizontalSlider() override = default;

  void setProps(const HorizontalSliderProps& _props);
  HorizontalSliderProps& getProps();
  const HorizontalSliderProps& getProps() const;
  void setPos(int x, int y) override;
  void setScale(float scale) override;
  void increment();
  void decrement();
  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;
  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/SectionScrollable.h ---
namespace ui {

// SectionScrollable-specific properties
struct SectionScrollableProps {
  int width = 0;
  int height = 0;
  int scrollBarWidth = 32;
  SDL_Color borderColor = Colors::Transparent;
  SDL_Color bgColor = Colors::Transparent;
  int borderSize = 0;
  int scrollStep = 20;
  int indicatorHeight = 20;
};

// SectionScrollable element - renders a scrollable section with inner/outer quads and
// scroll buttons Uses Position, Size, Scale from BaseStyle
class SectionScrollable : public UiElement {
private:
  SectionScrollableProps props;
  int scrollOffset = 0;    // Current scroll position
  int maxScrollOffset = 0; // Maximum scroll position
  int innerHeightScaled = 0;
  bool isDraggingIndicator = false;

  bmin::UniquePtr<Quad> outerQuad;
  Quad* innerQuad = nullptr;

  int getScrollIndicatorY(int offset) const;
  void updateScrollIndicatorPosition();
  void updateScrollButtonStates();
  void scrollFromIndicatorMouseY(int mouseY);
  bool isInScrollTrack(int mouseX, int mouseY) const;
  bool hitScrollIndicator(int mouseX, int mouseY);
  bool hitScrollButton(int mouseX, int mouseY);

public:
  SectionScrollable(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SectionScrollable() override = default;

  // Setters and getters for section-specific properties
  void setProps(const SectionScrollableProps& _props);
  SectionScrollableProps& getProps();
  const SectionScrollableProps& getProps() const;

  std::pair<int, int> getContentDims() const;

  // Content lives under outerQuad/innerQuad (not in children); search there too.
  UiElement* getChildById(std::string_view searchId) override;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseWheelEvent(int mouseX,
                            int mouseY,
                            int delta,
                            bmin::DynArray<UiElement*> additionalElements = {}) override;

  // Scroll methods
  void scrollUp();
  void scrollDown();
  void scrollTo(int offset);

  void addChild(UiElement* child) override;

  void setPos(int x, int y) override;
  void setScale(float scale) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonGroup.h ---
namespace ui {

enum class ButtonGroupAlignment { LEFT, CENTER, RIGHT };
enum class ButtonGroupButtonType { MODAL, SPRITE };

struct ButtonGroupButtonProps {
  bmin::String label;
  ButtonGroupButtonType type = ButtonGroupButtonType::MODAL;

  bmin::String spriteName;
  int spriteWidth = 16;
  int spriteHeight = 16;
  int spritePadding = 2;
  bool isSelected = false;
};
struct ButtonGroupProps {
  int width = 0;
  ButtonGroupAlignment alignment = ButtonGroupAlignment::LEFT;
  int buttonWidth = 80;
  int buttonHeight = 32;
  int buttonSpacing = 8; // Spacing between buttons
  int padding = 2;       // Inset around buttons; included in group width/height
  bmin::DynArray<ButtonGroupButtonProps> buttons;

  SDL_Color spriteBgColor = Colors::ButtonModalGrey1;
  SDL_Color spriteBgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color spriteBgColorBottomLeft = Colors::ButtonModalGrey3;
  int spriteBorderSize = 2;

  SDL_Color spriteSelectedBgColor = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorBottomLeft = Colors::ButtonModalSelected;
  int spriteSelectedBorderSize = 2;
};

class ButtonGroup : public UiElement {
private:
  ButtonGroupProps props;

public:
  ButtonGroup(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonGroup() override = default;

  void setProps(const ButtonGroupProps& _props);
  ButtonGroupProps& getProps();
  const ButtonGroupProps& getProps() const;

  void addObserverToButtonAtIndex(int index, UiEventObserver* observer);

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/elements/buttons/ButtonTextWrap.h ---
namespace ui {

// ButtonTextWrap-specific properties
struct ButtonTextWrapProps {
  int verticalPadding = 0; // Padding added to top and bottom
  int horizontalPadding = 0; // Padding added to left and right
  bool isSelected = false;
  // Forwarded to the internal TextParagraph (width wraps text; height grows to fit).
  TextParagraphProps textParagraph;
};

// ButtonTextWrap element - renders a clickable quad with wrapped text that changes color on hover
// Uses Position, Size, Scale from BaseStyle
class ButtonTextWrap : public UiElement {
private:
  ButtonTextWrapProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonTextWrap(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonTextWrap() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonTextWrapProps& _props);
  ButtonTextWrapProps& getProps();
  const ButtonTextWrapProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/borders/BorderDropShadow.h ---
namespace ui {

struct BorderDropShadowProps {
  int width = 0;
  int height = 0;
  SDL_Color backgroundColor = Colors::White;
  SDL_Color shadowColor = Colors::Black;
  int shadowOffsetX = -8;
  int shadowOffsetY = 8;
  int borderSize = 2;
  bool isSelected = false;
};

// BorderDropShadow - panel with fill, offset drop shadow, and outline border.
// Content children are laid out in logical space inside an internal scaled Quad.
class BorderDropShadow : public UiElement {
  BorderDropShadowProps props;

public:
  BorderDropShadow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderDropShadow() override = default;

  void setProps(const BorderDropShadowProps& _props);
  BorderDropShadowProps& getProps();
  const BorderDropShadowProps& getProps() const;

  void addChild(UiElement* child) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/borders/BorderInGame.h ---
namespace ui {

struct BorderInGameProps {
  int titleHeight = 44;
  int outsetBorderSize = 4;
  float actionButtonsScale = 1.f;
};

// BorderInGame component - base class for in-game border layouts using OutsetRectangle
// elements. Uses Position, Size, Scale from BaseStyle
class BorderInGame : public UiElement {
public:
  static constexpr int ACTION_BUTTON_SIZE = 32;

protected:
  virtual const BorderInGameProps& inGameProps() const = 0;

  int scaledWidth() const;
  int scaledHeight() const;
  void addOutsetRect(int x, int y, int width, int height);

public:
  BorderInGame(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGame() override = default;

  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getTitleDims() const;

  virtual const std::pair<int, int> getContentAreaLocation() const = 0;
  virtual const std::pair<int, int> getContentDims() const = 0;
  virtual const std::pair<int, int> getPartyMemberAreaLocation() const = 0;
  virtual const std::pair<int, int> getActionButtonsAreaLocation() const = 0;

  void render(int dt) override;
};

} // namespace ui

} // export

export {

// --- from ui/components/TiledOverlay.h ---
#if defined(MIYOOA30) || defined(MIYOOMINI)

#else

#endif

namespace ui {

struct TiledOverlayProps {
  int width = 0;
  int height = 0;
  bmin::String spriteName = "ui_overlay_256";
  int alpha = 40;
};

// Tiles a sprite into a Quad-sized offscreen texture (edges clip to the texture),
// then blits it with alpha. Decorative — does not take input.
class TiledOverlay : public UiElement {
private:
  TiledOverlayProps props;
  SDL_Texture* renderTexture = nullptr;
  int currentWidth = 0;
  int currentHeight = 0;

  void createRenderTexture();
  void destroyRenderTexture();

public:
  TiledOverlay(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TiledOverlay() override;

  void setProps(const TiledOverlayProps& _props);
  TiledOverlayProps& getProps();
  const TiledOverlayProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseWheelEvent(int mouseX,
                            int mouseY,
                            int delta,
                            bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

