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
 * @file paint_application.h
 * @brief A paint program for the textures of the 3D models.
 */

#pragma once

#include "common.h"
#include "renderer/patch/texture_editor.h"
#include "ui/application.h"
#include "ui/widget/button.h"

namespace ui {
struct Texture;

/**
 * @brief Draws on the texture of a 3D model with the stylus.
 *
 * The bottom screen shows the texture. The stylus draws on it, and the model
 * on the top screen changes at once, because the application writes the
 * pixels in the memory that the GPU reads. The right side has three tabs:
 * - Draw: the brush (size, tool, mirror), undo and reset.
 * - Color: changes the hue, the saturation and the brightness of all the
 *   texture, and replaces a color.
 * - File: saves the texture, writes it to a TGA file on the SD card, reads it
 *   again, and reloads it each time that the file changes (so the player can
 *   work on a computer).
 */
class PaintApplication : public Application {
  MAKE_SINGLETON(PaintApplication)

public:
  /**
   * @brief Opens the paint program on the texture of a unit of a material.
   * @param error Receives a short sentence when the function returns false.
   */
  static bool Open(const renderer::H3dModel* model, u32 material, u32 unit,
                   const c8** error);

  void Update(sys::Controller& controller) override;
  void DrawTop(sys::Graphics& graphics) override {}
  void DrawBottom(sys::Graphics& graphics) override;

private:
  enum Tool : u8 { kPaint, kPick, kErase, kFill, kToolCount };
  enum Tab : u8 { kTabDraw, kTabColor, kTabFile, kTabCount, kAllTabs = kTabCount };
  enum ButtonId : u8 {
    // The tab Draw.
    kSizeDown,
    kSizeUp,
    kToolButton,
    kMirror,
    kDrawUndo,
    kDrawReset,
    // The tab Color.
    kHueDown,
    kHueUp,
    kSaturationDown,
    kSaturationUp,
    kValueDown,
    kValueUp,
    kReplace,
    kColorUndo,
    // The tab File.
    kExport,
    kImport,
    kLive,
    kSave,
    kForget,
    kFileUndo,
    kFileReset,
    // All the tabs.
    kTabButton,
    kDone,
    kButtonCount
  };

  static constexpr u32 kPaletteSize = 16;
  static constexpr u32 kMaxBrush = 24;

  /// Writes all the pixels of the texture to the canvas.
  void RedrawCanvas();
  /// Writes one pixel of the canvas, from the texture.
  void CopyPixelToCanvas(u32 x, u32 y);
  /// Draws a disc of the brush at a pixel of the texture (and at the mirror
  /// pixel when the mirror is on).
  void Stamp(s32 x, s32 y);
  void StampOne(s32 x, s32 y);
  /// Draws the brush on a line, so that a fast stylus leaves no gaps.
  void Stroke(s32 from_x, s32 from_y, s32 to_x, s32 to_y);
  void UseTool(s32 x, s32 y, bool is_start);
  /// Fills the area of the same color with the current color.
  void Fill(s32 x, s32 y);
  /// Changes the hue (degrees), the saturation and the brightness of all the
  /// pixels.
  void AdjustColors(f32 hue, f32 saturation, f32 value);
  /// Replaces the color that the player picked by the current color. The
  /// shades stay.
  void ReplacePickedColor();
  void SaveUndo();
  void RestoreUndo();
  void RestoreOriginal();
  /// Writes the texture to the TGA file. Returns true on success.
  bool ExportFile();
  /// Reads a TGA file (24 or 32 bits, same size) into the texture.
  bool ImportBuffer(const u8* data, u32 size);
  bool ImportFile();
  /// Reads the TGA file again when it changed.
  void PollLiveFile();
  bool IsButtonActive(ButtonId id) const;
  bool IsPressed(ButtonId id) const;
  void SetMessage(const c16* text);
  void SelectTab(Tab tab);
  void Close();
  /// The rectangle of the canvas on the bottom screen.
  void GetCanvasRect(s32& x, s32& y, s32& width, s32& height) const;

  renderer::TextureTarget target_;
  Texture* canvas_ = nullptr;
  Button buttons_[kButtonCount];
  u8 color_[4] = {255, 0, 0, 255};
  u8 picked_color_[3] = {0, 0, 0};
  bool has_picked_color_ = false;
  u8 brush_size_ = 4;
  Tool tool_ = kPaint;
  Tab tab_ = kTabDraw;
  bool is_mirror_ = false;
  bool was_down_ = false;
  bool is_dirty_ = false;
  bool has_copies_ = false;
  u32 original_hash_ = 0; ///< The hash of the original pixels, for the saved file.
  bool is_saved_ = false; ///< The original texture has a saved file.
  const c16* message_ = nullptr; ///< A short text that replaces the size.
  u32 message_frames_ = 0;
  bool is_live_ = false; ///< The program reloads the TGA file when it changes.
  u32 live_frames_ = 0;
  u32 live_hash_ = 0;
  /// 0: the canvas has the size of the texture. 1: half the size, when the
  /// memory of the GPU is short.
  u32 canvas_shift_ = 0;
  s32 last_x_ = 0;
  s32 last_y_ = 0;
};
} // namespace ui
