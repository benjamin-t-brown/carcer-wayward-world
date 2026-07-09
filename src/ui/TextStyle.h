#pragma once

#include "sdl2w/Defines.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/colors.h"

namespace ui {

enum class FontFamily { TEXT, TEXT_BOLD, DEFAULT, TITLE };

enum class TextAlign { LEFT_TOP, LEFT_CENTER, LEFT_BOTTOM, CENTER };

struct TextFontProps {
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
  TextAlign textAlign = TextAlign::LEFT_TOP;
};

enum class BaseFontConfig {
  MODAL_TEXT,
  MODAL_TEXT_BOLD,
  MODAL_TITLE,
  MODAL_CHOICE_TEXT,
  MODAL_BUTTON,
};

inline void setBaseFontConfig(TextFontProps& font, BaseFontConfig config) {
  switch (config) {
  case BaseFontConfig::MODAL_TEXT:
    font.fontFamily = FontFamily::TEXT;
    font.fontSize = sdl2w::TEXT_SIZE_16;
    font.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_TEXT_BOLD:
    font.fontFamily = FontFamily::TEXT_BOLD;
    font.fontSize = sdl2w::TEXT_SIZE_16;
    font.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_TITLE:
    font.fontFamily = FontFamily::TITLE;
    font.fontSize = sdl2w::TEXT_SIZE_20;
    font.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_CHOICE_TEXT:
    font.fontFamily = FontFamily::TEXT;
    font.fontSize = sdl2w::TEXT_SIZE_24;
    font.fontColor = Colors::Black;
    break;
  case BaseFontConfig::MODAL_BUTTON:
    font.fontFamily = FontFamily::TEXT;
    font.fontSize = sdl2w::TEXT_SIZE_20;
    font.fontColor = Colors::White;
    break;
  }
}

} // namespace ui
