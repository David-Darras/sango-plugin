/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file color_picker.cc
 * @brief A color picker for the bottom screen.
 *
 * The declarations are in ui/widget/color_picker.h.
 */

#include "ui/widget/color_picker.h"

#include "core/utils.h"
#include "system/native/graphics.h"
#include "system/native/touch_screen.h"
#include "ui/theme.h"

namespace ui {
namespace {
// The textures: a power of 2 for each side. The square is drawn larger than
// its texture; the colors change slowly, so the player does not see it.
constexpr u32 kSquareTexture = 64;
constexpr u32 kBarTextureWidth = 8;
constexpr u32 kBarTextureHeight = 128;

// The layout, from the top-left corner of the square.
constexpr s32 kHueBarX = ColorPicker::kX + 126;
constexpr s32 kAlphaBarX = ColorPicker::kX + 148;
constexpr s32 kNewSwatchHeight = 28;
constexpr s32 kOldSwatchHeight = 14;
constexpr s32 kHexLineY = ColorPicker::kY + 46;
constexpr s32 kRgbLineY = ColorPicker::kY + 60;
constexpr s32 kPaletteY = ColorPicker::kY + 76;
constexpr s32 kPaletteColumns = 6;
constexpr s32 kPaletteCellWidth = 19;
constexpr s32 kPaletteCellHeight = 16;
constexpr s32 kPaletteGap = 2;

// The colors of the palette: red, green, blue.
const u8 kPalette[12][3] = {
    {255, 255, 255}, {192, 192, 192}, {128, 128, 128}, {0, 0, 0},
    {255, 0, 0},     {255, 128, 0},   {255, 255, 0},   {0, 255, 0},
    {0, 255, 255},   {0, 0, 255},     {128, 0, 255},   {255, 0, 160},
};

f32 Clamp(f32 value) {
  return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

// Hue 0 to 360, saturation and brightness 0 to 1.
void HsvToRgb(f32 hue, f32 saturation, f32 value, f32& r, f32& g, f32& b) {
  const f32 sector = hue / 60.0f;
  const s32 index = (s32)sector;
  const f32 fraction = sector - (f32)index;
  const f32 p = value * (1.0f - saturation);
  const f32 q = value * (1.0f - saturation * fraction);
  const f32 t = value * (1.0f - saturation * (1.0f - fraction));
  switch (index % 6) {
    case 0: r = value; g = t; b = p; break;
    case 1: r = q; g = value; b = p; break;
    case 2: r = p; g = value; b = t; break;
    case 3: r = p; g = q; b = value; break;
    case 4: r = t; g = p; b = value; break;
    default: r = value; g = p; b = q; break;
  }
}

u8 ToByte(f32 value) { return (u8)(Clamp(value) * 255.0f + 0.5f); }

// Returns true when a point is inside a rectangle.
bool IsInside(s32 x, s32 y, s32 left, s32 top, s32 width, s32 height) {
  return x >= left && x < left + width && y >= top && y < top + height;
}

// Writes the character of a digit of a hexadecimal number.
c16 GetHexDigit(u32 value) {
  return value < 10 ? (c16)(u'0' + value) : (c16)(u'A' + value - 10);
}
} // namespace

void ColorPicker::Begin(const Color& color) {
  start_ = color;
  start_.r = Clamp(start_.r);
  start_.g = Clamp(start_.g);
  start_.b = Clamp(start_.b);
  start_.a = Clamp(start_.a);
  SetRgb(start_.r, start_.g, start_.b);
  alpha_ = start_.a;
  is_square_dirty_ = true;
  is_alpha_dirty_ = true;
  ResetTouch();
}

void ColorPicker::SetRgb(f32 r, f32 g, f32 b) {
  const f32 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
  const f32 min = r < g ? (r < b ? r : b) : (g < b ? g : b);
  const f32 delta = max - min;
  value_ = max;
  // A gray color has no hue, and black has no saturation: the picker keeps
  // the last values, so the bars do not jump.
  if (max > 0.0f) saturation_ = delta / max;
  if (delta <= 0.0f) return;
  f32 hue;
  if (max == r) {
    hue = (g - b) / delta;
    if (hue < 0.0f) hue += 6.0f;
  } else if (max == g) {
    hue = (b - r) / delta + 2.0f;
  } else {
    hue = (r - g) / delta + 4.0f;
  }
  hue_ = hue * 60.0f;
}

void ColorPicker::Sync(const Color& color) {
  const f32 r = Clamp(color.r);
  const f32 g = Clamp(color.g);
  const f32 b = Clamp(color.b);
  const f32 a = Clamp(color.a);
  // The picker writes a byte color with a small rounding: it accepts half a
  // step of a byte.
  constexpr f32 kTolerance = 0.6f / 255.0f;
  f32 current_r;
  f32 current_g;
  f32 current_b;
  HsvToRgb(hue_, saturation_, value_, current_r, current_g, current_b);
  const f32 dr = current_r - r;
  const f32 dg = current_g - g;
  const f32 db = current_b - b;
  const f32 da = alpha_ - a;
  const bool is_same = dr < kTolerance && dr > -kTolerance &&
                       dg < kTolerance && dg > -kTolerance &&
                       db < kTolerance && db > -kTolerance &&
                       da < kTolerance && da > -kTolerance;
  if (is_same) return;
  const f32 old_hue = hue_;
  SetRgb(r, g, b);
  alpha_ = a;
  if (hue_ != old_hue) is_square_dirty_ = true;
  is_alpha_dirty_ = true;
}

void ColorPicker::ResetTouch() {
  drag_ = kDragNone;
  is_started_ = false;
  // A touch that is in progress does not start a gesture here.
  was_down_ = true;
}

Color ColorPicker::GetColor() const {
  Color color;
  HsvToRgb(hue_, saturation_, value_, color.r, color.g, color.b);
  color.a = alpha_;
  return color;
}

void ColorPicker::ApplyTouch(s32 x, s32 y) {
  constexpr s32 kLast = kSquareSize - 1;
  s32 position = y - kY;
  if (position < 0) position = 0;
  if (position > kLast) position = kLast;
  const f32 along = (f32)position / (f32)kLast;
  switch (drag_) {
    case kDragSquare: {
      s32 column = x - kX;
      if (column < 0) column = 0;
      if (column > kLast) column = kLast;
      saturation_ = (f32)column / (f32)kLast;
      value_ = 1.0f - along;
      is_alpha_dirty_ = true;
      break;
    }
    case kDragHue:
      hue_ = along * 359.99f;
      is_square_dirty_ = true;
      is_alpha_dirty_ = true;
      break;
    case kDragAlpha:
      alpha_ = 1.0f - along;
      break;
    default:
      break;
  }
}

void ColorPicker::GetPaletteCell(u32 index, s32& x, s32& y) const {
  const s32 column = (s32)index % kPaletteColumns;
  const s32 row = (s32)index / kPaletteColumns;
  x = kPanelX + column * (kPaletteCellWidth + kPaletteGap);
  y = kPaletteY + row * (kPaletteCellHeight + kPaletteGap);
}

bool ColorPicker::Update() {
  is_started_ = false;
  sys::TouchScreen& touch_screen = sys::TouchScreen::GetInstance();
  const bool is_down = touch_screen.IsDown();
  bool is_changed = false;
  if (!is_down) {
    drag_ = kDragNone;
    was_down_ = false;
    return false;
  }
  const s32 x = touch_screen.GetX();
  const s32 y = touch_screen.GetY();

  // A new touch chooses what it does until the stylus leaves the screen.
  if (!was_down_) {
    if (IsInside(x, y, kX, kY, kSquareSize, kSquareSize)) {
      drag_ = kDragSquare;
    } else if (IsInside(x, y, kHueBarX, kY, kBarWidth, kSquareSize)) {
      drag_ = kDragHue;
    } else if (IsInside(x, y, kAlphaBarX, kY, kBarWidth, kSquareSize)) {
      drag_ = kDragAlpha;
    } else if (IsInside(x, y, kPanelX, kY + kNewSwatchHeight, kPanelWidth,
                        kOldSwatchHeight)) {
      // The old swatch puts the color of the start back.
      SetRgb(start_.r, start_.g, start_.b);
      alpha_ = start_.a;
      is_square_dirty_ = true;
      is_alpha_dirty_ = true;
      is_changed = true;
    } else {
      for (u32 i = 0; i < kPaletteCount; i++) {
        s32 cell_x;
        s32 cell_y;
        GetPaletteCell(i, cell_x, cell_y);
        if (!IsInside(x, y, cell_x, cell_y, kPaletteCellWidth,
                      kPaletteCellHeight)) {
          continue;
        }
        // A palette color keeps the alpha of the color.
        SetRgb(kPalette[i][0] / 255.0f, kPalette[i][1] / 255.0f,
               kPalette[i][2] / 255.0f);
        is_square_dirty_ = true;
        is_alpha_dirty_ = true;
        is_changed = true;
        break;
      }
    }
    is_started_ = drag_ != kDragNone || is_changed;
  }
  was_down_ = true;
  if (drag_ != kDragNone) {
    ApplyTouch(x, y);
    is_changed = true;
  }
  return is_changed;
}

void ColorPicker::UpdateTextures() {
  if (!is_textures_tried_) {
    is_textures_tried_ = true;
    square_ = Texture::Create(kSquareTexture, kSquareTexture);
    hue_bar_ = Texture::Create(kBarTextureWidth, kBarTextureHeight);
    alpha_bar_ = Texture::Create(kBarTextureWidth, kBarTextureHeight);
    // The hue bar does not change.
    if (hue_bar_ != nullptr) {
      for (u32 y = 0; y < kBarTextureHeight; y++) {
        f32 r;
        f32 g;
        f32 b;
        HsvToRgb(359.99f * y / (kBarTextureHeight - 1), 1.0f, 1.0f, r, g, b);
        for (u32 x = 0; x < kBarTextureWidth; x++) {
          hue_bar_->SetPixel(x, y, ToByte(r), ToByte(g), ToByte(b), 255);
        }
      }
      hue_bar_->Flush();
    }
  }

  if (is_square_dirty_ && square_ != nullptr) {
    is_square_dirty_ = false;
    for (u32 y = 0; y < kSquareTexture; y++) {
      const f32 value = 1.0f - (f32)y / (f32)(kSquareTexture - 1);
      for (u32 x = 0; x < kSquareTexture; x++) {
        f32 r;
        f32 g;
        f32 b;
        HsvToRgb(hue_, (f32)x / (f32)(kSquareTexture - 1), value, r, g, b);
        square_->SetPixel(x, y, ToByte(r), ToByte(g), ToByte(b), 255);
      }
    }
    square_->Flush();
  }

  if (is_alpha_dirty_ && alpha_bar_ != nullptr) {
    is_alpha_dirty_ = false;
    f32 r;
    f32 g;
    f32 b;
    HsvToRgb(hue_, saturation_, value_, r, g, b);
    for (u32 y = 0; y < kBarTextureHeight; y++) {
      const f32 alpha = 1.0f - (f32)y / (f32)(kBarTextureHeight - 1);
      for (u32 x = 0; x < kBarTextureWidth; x++) {
        // A checkerboard under the color shows the alpha.
        const f32 back = (((x / 4) + (y / 8)) & 1) != 0 ? 0.55f : 0.8f;
        alpha_bar_->SetPixel(x, y, ToByte(r * alpha + back * (1.0f - alpha)),
                             ToByte(g * alpha + back * (1.0f - alpha)),
                             ToByte(b * alpha + back * (1.0f - alpha)), 255);
      }
    }
    alpha_bar_->Flush();
  }
}

void ColorPicker::Draw() {
  UpdateTextures();
  const Theme& theme = Theme::GetInstance();
  const Color white(1, 1, 1, 1);
  const Color black(0, 0, 0, 1);
  const Color color = GetColor();
  f32 hue_r;
  f32 hue_g;
  f32 hue_b;
  HsvToRgb(hue_, 1.0f, 1.0f, hue_r, hue_g, hue_b);

  // The square, the hue bar and the alpha bar: one rectangle for each.
  if (square_ != nullptr) {
    sys::Graphics::DrawRectWithTexture(kX, kY, kSquareSize, kSquareSize, white,
                                       square_);
  } else {
    sys::Graphics::DrawRect(kX, kY, kSquareSize, kSquareSize,
                            Color(hue_r, hue_g, hue_b, 1));
  }
  if (hue_bar_ != nullptr) {
    sys::Graphics::DrawRectWithTexture(kHueBarX, kY, kBarWidth, kSquareSize,
                                       white, hue_bar_);
  }
  if (alpha_bar_ != nullptr) {
    sys::Graphics::DrawRectWithTexture(kAlphaBarX, kY, kBarWidth, kSquareSize,
                                       white, alpha_bar_);
  }

  // The marks: a cross in the square, a line on each bar.
  constexpr s32 kLast = kSquareSize - 1;
  const s32 mark_x = kX + (s32)(saturation_ * kLast + 0.5f);
  const s32 mark_y = kY + (s32)((1.0f - value_) * kLast + 0.5f);
  sys::Graphics::DrawRect(mark_x - 6, mark_y - 1, 13, 3, black);
  sys::Graphics::DrawRect(mark_x - 1, mark_y - 6, 3, 13, black);
  sys::Graphics::DrawRect(mark_x - 5, mark_y, 11, 1, white);
  sys::Graphics::DrawRect(mark_x, mark_y - 5, 1, 11, white);
  const s32 hue_y = kY + (s32)(hue_ / 359.99f * kLast + 0.5f);
  sys::Graphics::DrawRect(kHueBarX - 2, hue_y - 2, kBarWidth + 4, 5, black);
  sys::Graphics::DrawRect(kHueBarX - 1, hue_y - 1, kBarWidth + 2, 3, white);
  const s32 alpha_y = kY + (s32)((1.0f - alpha_) * kLast + 0.5f);
  sys::Graphics::DrawRect(kAlphaBarX - 2, alpha_y - 2, kBarWidth + 4, 5,
                          black);
  sys::Graphics::DrawRect(kAlphaBarX - 1, alpha_y - 1, kBarWidth + 2, 3,
                          white);

  // The swatches: the new color, then the color of the start.
  sys::Graphics::DrawRect(kPanelX, kY, kPanelWidth, kNewSwatchHeight,
                          Color(color.r, color.g, color.b, 1));
  sys::Graphics::DrawRect(kPanelX, kY + kNewSwatchHeight, kPanelWidth,
                          kOldSwatchHeight,
                          Color(start_.r, start_.g, start_.b, 1));

  // The code and the values: two texts.
  const u8 r = ToByte(color.r);
  const u8 g = ToByte(color.g);
  const u8 b = ToByte(color.b);
  const u8 a = ToByte(color.a);
  const u8 bytes[4] = {r, g, b, a};
  c16 code[16];
  u32 length = 0;
  code[length++] = u'#';
  for (u32 i = 0; i < 4; i++) {
    code[length++] = GetHexDigit(bytes[i] >> 4);
    code[length++] = GetHexDigit(bytes[i] & 15);
  }
  code[length] = 0;
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  DrawMenuText(kPanelX, kHexLineY, code, theme.selected_text_color);
  c16 values[48];
  core::Utils::Format(values, u"R %u  G %u  B %u  A %u", r, g, b, a);
  sys::Graphics::SetTextScale(0.42f, 0.42f);
  DrawMenuText(kPanelX, kRgbLineY, values, theme.unselected_text_color);

  // The palette.
  for (u32 i = 0; i < kPaletteCount; i++) {
    s32 cell_x;
    s32 cell_y;
    GetPaletteCell(i, cell_x, cell_y);
    sys::Graphics::DrawRect(
        cell_x, cell_y, kPaletteCellWidth, kPaletteCellHeight,
        Color(kPalette[i][0] / 255.0f, kPalette[i][1] / 255.0f,
              kPalette[i][2] / 255.0f, 1));
  }
}
} // namespace ui
