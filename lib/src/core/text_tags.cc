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
 * @file text_tags.cc
 * @brief Adds tags to the texts of the game: any colour, a gradient, a size.
 *
 * A tag of a text is `0x0010`, the number of the next values, the code of
 * the tag, then its parameters. The code of the game skips a tag of the
 * group 0xBD that it does not know, so a text with these tags shows
 * correctly without the plugin (without the effect). Sango Studio writes
 * them (`[color #FF8000]`, `[gradient #FF0000 #0000FF]`, `[size 150]`).
 *
 * | Code   | Parameters                         | Effect on the next letters  |
 * | ------ | ---------------------------------- | --------------------------- |
 * | 0xBD10 | red, green, blue (0 to 255)        | This colour.                |
 * | 0xBD11 | red, green, blue of the top, then of the bottom | A gradient from the top to the bottom. |
 * | 0xBD12 | the size in percent                | This size.                  |
 * | 0xBD13 |                                    | The normal size again.      |
 *
 * The tag of the game 0xBD01 (`[/color]`) also ends 0xBD10 and 0xBD11.
 *
 * The plugin hooks the function of the text boxes that does the tags while
 * a text is drawn. It changes the colours with the same fields as the
 * colour tags of the game, so that the game puts the colour back at
 * 0xBD01.
 *
 * Not tested in the game yet. The width that the game calculates for a line
 * (to put it in the middle or on the right) does not count the size.
 */

#include "common.h"
#include "core/hook.h"
#include "system/address.h"

namespace core {
namespace {

/// The character that starts a tag.
constexpr u16 kTagStart = 0x0010;
/// The group of the general tags.
constexpr u16 kGeneralGroup = 0xBD;
constexpr u16 kColorEnd = 0x01;
constexpr u16 kRgb = 0x10;
constexpr u16 kGradient = 0x11;
constexpr u16 kSize = 0x12;
constexpr u16 kSizeEnd = 0x13;

/// The value that the function returns for a tag that takes no space.
constexpr u32 kNoSpace = 1;

// The fields of a text writer.
constexpr u32 kWriterTopColor = 0x18;    ///< u32: red, green, blue, alpha.
constexpr u32 kWriterBottomColor = 0x1C; ///< u32: red, green, blue, alpha.
constexpr u32 kWriterGradient = 0x20;    ///< u8: 0 none, 1 left-right, 2 top-bottom.
constexpr u32 kWriterScaleX = 0x24;      ///< f32.
constexpr u32 kWriterScaleY = 0x28;      ///< f32.
constexpr u8 kGradientTopBottom = 2;

// The fields of the object of a text box that the colour tags of the game use.
constexpr u32 kBoxSavedTop = 0x04;     ///< u32: the colour before the tag.
constexpr u32 kBoxSavedBottom = 0x08;  ///< u32: the colour before the tag.
constexpr u32 kBoxTop = 0x0C;          ///< u32: the colour of the tag.
constexpr u32 kBoxBottom = 0x10;       ///< u32: the colour of the tag.
constexpr u32 kBoxColorChanged = 0x1C; ///< u8: 1 when a tag changed the colour.
constexpr u32 kBoxLastText = 0x24;     ///< The furthest character that was done.

/// The state of the drawing of a text: the first field is the text writer,
/// the second the next character.
struct DrawState {
  u8* writer;
  const u16* text;
};

/// What the plugin changed in a text writer, to put it back.
struct WriterChange {
  u8* writer = nullptr;
  u8 gradient = 0;
  bool has_gradient = false;
  float scale_x = 1.0f;
  float scale_y = 1.0f;
  bool has_scale = false;
};

WriterChange change;

template <typename T> T& Field(u8* object, u32 offset) {
  return *reinterpret_cast<T*>(object + offset);
}

/// A colour of the writer from the parameters red, green, blue.
u32 Color(const u16* values) {
  return (values[0] & 0xFF) | (values[1] & 0xFF) << 8 |
         (values[2] & 0xFF) << 16 | 0xFF000000u;
}

void UpdateColors(u8* writer) {
  reinterpret_cast<void (*)(u8*)>(sys::address::kTextWriterUpdateColors)(
      writer);
}

/// Gives the colours to the writer, like the colour tags of the game do.
void SetColors(u8* box, u8* writer, u32 top, u32 bottom, u8 gradient) {
  if (Field<u8>(box, kBoxColorChanged) == 0) {
    Field<u32>(box, kBoxSavedTop) = Field<u32>(writer, kWriterTopColor);
    Field<u32>(box, kBoxSavedBottom) = Field<u32>(writer, kWriterBottomColor);
  }
  if (!change.has_gradient || change.writer != writer) {
    change.writer = writer;
    change.gradient = Field<u8>(writer, kWriterGradient);
    change.has_gradient = true;
  }
  Field<u32>(box, kBoxTop) = top;
  Field<u32>(box, kBoxBottom) = bottom;
  Field<u32>(writer, kWriterTopColor) = top;
  Field<u32>(writer, kWriterBottomColor) = bottom;
  Field<u8>(writer, kWriterGradient) = gradient;
  UpdateColors(writer);
  Field<u8>(box, kBoxColorChanged) = 1;
}

/// Changes the size of the next letters, in percent of the size before the
/// first size tag.
void SetSize(u8* writer, u16 percent) {
  if (!change.has_scale || change.writer != writer) {
    change.writer = writer;
    change.scale_x = Field<float>(writer, kWriterScaleX);
    change.scale_y = Field<float>(writer, kWriterScaleY);
    change.has_scale = true;
  }
  float ratio = static_cast<float>(percent) / 100.0f;
  Field<float>(writer, kWriterScaleX) = change.scale_x * ratio;
  Field<float>(writer, kWriterScaleY) = change.scale_y * ratio;
}

void EndSize(u8* writer) {
  if (!change.has_scale || change.writer != writer) return;
  Field<float>(writer, kWriterScaleX) = change.scale_x;
  Field<float>(writer, kWriterScaleY) = change.scale_y;
  change.has_scale = false;
}

} // namespace

HOOK(u32, DoTextTag, (u8* box, u16 code, DrawState* state),
     sys::address::kDoTextTag) {
  const u16* tag = state->text - 1;
  if (*tag != kTagStart) return original(box, code, state);
  u16 count = tag[1];
  u16 tag_code = tag[2];
  if ((tag_code >> 8) != kGeneralGroup) return original(box, code, state);
  u16 index = tag_code & 0xFF;
  const u16* params = tag + 3;
  u16 param_count = count > 0 ? count - 1 : 0;
  u8* writer = state->writer;

  if (index == kColorEnd) {
    // The game puts the colours back; the plugin puts the gradient mode back.
    u32 result = original(box, code, state);
    if (change.has_gradient && change.writer == writer) {
      Field<u8>(writer, kWriterGradient) = change.gradient;
      change.has_gradient = false;
      UpdateColors(writer);
    }
    return result;
  }
  if (index < kRgb || index > kSizeEnd) return original(box, code, state);

  if (index == kRgb && param_count >= 3) {
    u32 color = Color(params);
    SetColors(box, writer, color, color, change.writer == writer && change.has_gradient ? change.gradient : Field<u8>(writer, kWriterGradient));
  } else if (index == kGradient && param_count >= 6) {
    SetColors(box, writer, Color(params), Color(params + 3), kGradientTopBottom);
  } else if (index == kSize && param_count >= 1) {
    SetSize(writer, params[0]);
  } else if (index == kSizeEnd) {
    EndSize(writer);
  }

  // Like the game: keep the furthest character, then go after the tag.
  const u16*& last = Field<const u16*>(box, kBoxLastText);
  if (last < state->text) last = state->text;
  state->text = reinterpret_cast<const u16* (*)(const u16*)>(
      sys::address::kTextSkipTag)(tag);
  return kNoSpace;
}

} // namespace core
