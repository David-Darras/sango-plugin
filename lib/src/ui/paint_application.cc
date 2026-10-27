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
 * @file paint_application.cc
 * @brief A paint program for the textures of the 3D models.
 *
 * The declarations are in ui/paint_application.h.
 */

#include "ui/paint_application.h"

#include <cstring>

#include "core/game_file.h"
#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/touch_screen.h"
#include "ui/application_manager.h"
#include "ui/theme.h"
#include "ui/widget/texture.h"

namespace ui {

namespace {
using renderer::TextureEditor;

// The layout of the bottom screen: the texture on the left, the tools on the
// right.
constexpr s32 kCanvasSize = 240;
constexpr s32 kPanelX = 246;
constexpr s32 kSwatchSize = 17;
constexpr s32 kSwatchStep = 18;
constexpr s32 kSwatchesY = 4;
constexpr s32 kButtonWidth = 70;
constexpr s32 kButtonHeight = 22;

const u8 kPalette[16][3] = {
    {0, 0, 0},       {255, 255, 255}, {128, 128, 128}, {255, 0, 0},
    {255, 128, 0},   {255, 224, 0},   {128, 255, 0},   {0, 160, 0},
    {0, 224, 224},   {0, 96, 255},    {128, 0, 224},   {255, 0, 192},
    {128, 64, 0},    {255, 160, 192}, {255, 208, 160}, {64, 64, 96},
};

// The memory of the copies for the undo and the reset. The memory of the
// device heap stays for all the game: the program allocates it one time, for
// the biggest texture that it meets.
u8* undo_pixels = nullptr;
u8* original_pixels = nullptr;
u32 copy_capacity = 0;

// The canvases: one for each size of texture, made when the first texture of
// the size opens.
Texture* canvases[6][6] = {};

u32 Log2(u32 value) {
  u32 result = 0;
  while ((1u << result) < value) result++;
  return result;
}

Texture* GetCanvas(u32 width, u32 height) {
  const u32 column = Log2(width) - 3;
  const u32 row = Log2(height) - 3;
  if (column >= 6 || row >= 6) return nullptr;
  if (canvases[row][column] == nullptr) {
    canvases[row][column] = Texture::Create(width, height);
  }
  return canvases[row][column];
}

// Makes the copies for the undo and the reset. Without memory, the program
// works without them. Returns true when the copies exist.
bool EnsureCopies(u32 size) {
  if (copy_capacity >= size) return true;
  u8* undo = static_cast<u8*>(core::GameFile::AllocateDevice(size, 128));
  u8* original =
      undo != nullptr
          ? static_cast<u8*>(core::GameFile::AllocateDevice(size, 128))
          : nullptr;
  if (undo == nullptr || original == nullptr) return false;
  undo_pixels = undo;
  original_pixels = original;
  copy_capacity = size;
  return true;
}
} // namespace

bool PaintApplication::Open(const renderer::H3dModel* model, u32 material,
                            u32 unit, const c8** error) {
  auto& app = GetInstance();
  renderer::TextureTarget target;
  if (!TextureEditor::Locate(model, material, unit, target, error)) return false;
  // The canvas: the size of the texture, or half the size when the memory of
  // the GPU is short.
  u32 shift = 0;
  Texture* canvas = GetCanvas(target.width, target.height);
  if (canvas == nullptr && target.width >= 16 && target.height >= 16) {
    shift = 1;
    canvas = GetCanvas(target.width / 2, target.height / 2);
  }
  if (canvas == nullptr) {
    *error = "Not enough memory of the GPU for the canvas.";
    return false;
  }

  app.target_ = target;
  app.canvas_ = canvas;
  app.canvas_shift_ = shift;
  app.was_down_ = false;
  app.is_dirty_ = false;
  app.has_copies_ = EnsureCopies(target.GetSize());
  if (app.has_copies_) {
    memcpy(original_pixels, target.pixels, target.GetSize());
    memcpy(undo_pixels, target.pixels, target.GetSize());
  }
  app.RedrawCanvas();
  app.original_hash_ = TextureEditor::GetOriginalHash(target);
  app.is_saved_ = TextureEditor::IsSaved(target, app.original_hash_);
  app.message_frames_ = 0;

  const s32 x = kPanelX;
  app.buttons_[kSizeDown].Initialize(x, 100, 34, kButtonHeight);
  app.buttons_[kSizeUp].Initialize(x + 36, 100, 34, kButtonHeight);
  app.buttons_[kToolButton].Initialize(x, 126, kButtonWidth, kButtonHeight);
  app.buttons_[kUndo].Initialize(x, 152, 34, kButtonHeight);
  app.buttons_[kReset].Initialize(x + 36, 152, 34, kButtonHeight);
  app.buttons_[kSave].Initialize(x, 178, 34, kButtonHeight);
  app.buttons_[kForget].Initialize(x + 36, 178, 34, kButtonHeight);
  app.buttons_[kDone].Initialize(x, 210, kButtonWidth, kButtonHeight);
  for (Button& button : app.buttons_) button.Reset();

  ApplicationManager::GetInstance().Push(app);
  return true;
}

void PaintApplication::GetCanvasRect(s32& x, s32& y, s32& width,
                                     s32& height) const {
  // The texture keeps its shape and fits in the square.
  const s32 side = static_cast<s32>(target_.width > target_.height
                                        ? target_.width
                                        : target_.height);
  width = static_cast<s32>(target_.width) * kCanvasSize / side;
  height = static_cast<s32>(target_.height) * kCanvasSize / side;
  x = (kCanvasSize - width) / 2;
  y = (kCanvasSize - height) / 2;
}

void PaintApplication::CopyPixelToCanvas(u32 x, u32 y) {
  // A half-size canvas shows one pixel of two.
  if (((x | y) & ((1u << canvas_shift_) - 1)) != 0) return;
  u8 rgba[4];
  TextureEditor::ReadPixel(target_, x, y, rgba);
  canvas_->SetPixel(x >> canvas_shift_, y >> canvas_shift_, rgba[0], rgba[1],
                    rgba[2], rgba[3]);
}

void PaintApplication::RedrawCanvas() {
  for (u32 y = 0; y < target_.height; y++) {
    for (u32 x = 0; x < target_.width; x++) CopyPixelToCanvas(x, y);
  }
  canvas_->Flush();
}

void PaintApplication::Stamp(s32 x, s32 y) {
  const s32 radius_twice = brush_size_; // The radius is brush_size_ / 2.
  const s32 reach = (radius_twice + 1) / 2;
  u8 rgba[4] = {color_[0], color_[1], color_[2], color_[3]};
  if (tool_ == kErase) rgba[3] = 0;
  for (s32 dy = -reach; dy <= reach; dy++) {
    for (s32 dx = -reach; dx <= reach; dx++) {
      // The pixel is in the disc when its distance is not more than the
      // radius: (2 dx)^2 + (2 dy)^2 <= size^2.
      if (4 * (dx * dx + dy * dy) > radius_twice * radius_twice) continue;
      const s32 px = x + dx;
      const s32 py = y + dy;
      if (px < 0 || py < 0 || px >= static_cast<s32>(target_.width) ||
          py >= static_cast<s32>(target_.height)) {
        continue;
      }
      TextureEditor::WritePixel(target_, px, py, rgba);
      CopyPixelToCanvas(px, py);
    }
  }
  is_dirty_ = true;
}

void PaintApplication::Stroke(s32 from_x, s32 from_y, s32 to_x, s32 to_y) {
  s32 steps = to_x > from_x ? to_x - from_x : from_x - to_x;
  const s32 steps_y = to_y > from_y ? to_y - from_y : from_y - to_y;
  if (steps_y > steps) steps = steps_y;
  if (steps == 0) {
    Stamp(to_x, to_y);
    return;
  }
  // One stamp for each pixel of the longest side.
  for (s32 i = 0; i <= steps; i++) {
    Stamp(from_x + (to_x - from_x) * i / steps,
          from_y + (to_y - from_y) * i / steps);
  }
}

void PaintApplication::SaveUndo() {
  if (has_copies_) memcpy(undo_pixels, target_.pixels, target_.GetSize());
}

void PaintApplication::UseTool(s32 x, s32 y, bool is_start) {
  if (tool_ == kPick) {
    if (x < 0 || y < 0 || x >= static_cast<s32>(target_.width) ||
        y >= static_cast<s32>(target_.height)) {
      return;
    }
    u8 rgba[4];
    TextureEditor::ReadPixel(target_, x, y, rgba);
    color_[0] = rgba[0];
    color_[1] = rgba[1];
    color_[2] = rgba[2];
    color_[3] = 255;
    return;
  }
  if (is_start) {
    SaveUndo();
    Stamp(x, y);
  } else {
    Stroke(last_x_, last_y_, x, y);
  }
  last_x_ = x;
  last_y_ = y;
}

void PaintApplication::Close() {
  ApplicationManager::GetInstance().Pop();
}

void PaintApplication::Update(sys::Controller& controller) {
  if (!TextureEditor::IsValid(target_) ||
      controller.IsKeyPressed(Key::kB) || controller.IsKeyPressed(Key::kStart)) {
    Close();
    return;
  }
  for (Button& button : buttons_) button.Update();

  if (buttons_[kSizeDown].IsReleased() && brush_size_ > 1) brush_size_--;
  if (buttons_[kSizeUp].IsReleased() && brush_size_ < kMaxBrush) brush_size_++;
  if (buttons_[kToolButton].IsReleased()) {
    do {
      tool_ = static_cast<Tool>((tool_ + 1) % kToolCount);
    } while (tool_ == kErase && !TextureEditor::HasAlpha(target_.format));
  }
  if (buttons_[kUndo].IsReleased() && has_copies_) {
    memcpy(target_.pixels, undo_pixels, target_.GetSize());
    RedrawCanvas();
    is_dirty_ = true;
  }
  if (buttons_[kReset].IsReleased() && has_copies_) {
    memcpy(target_.pixels, original_pixels, target_.GetSize());
    RedrawCanvas();
    is_dirty_ = true;
  }
  if (buttons_[kSave].IsReleased()) {
    is_message_ok_ = TextureEditor::Save(target_, original_hash_);
    is_saved_ = is_saved_ || is_message_ok_;
    message_frames_ = 90;
  }
  if (buttons_[kForget].IsReleased()) {
    is_message_ok_ = TextureEditor::Forget(target_, original_hash_);
    is_saved_ = false;
    message_frames_ = 90;
  }
  if (message_frames_ != 0) message_frames_--;
  if (buttons_[kDone].IsReleased()) {
    Close();
    return;
  }

  sys::TouchScreen& touch = sys::TouchScreen::GetInstance();
  const bool is_down = touch.IsDown();
  const s32 touch_x = touch.GetX();
  const s32 touch_y = touch.GetY();

  // The palette.
  if (is_down && !was_down_ && touch_x >= kPanelX) {
    for (u32 i = 0; i < kPaletteSize; i++) {
      const s32 x = kPanelX + (i % 4) * kSwatchStep;
      const s32 y = kSwatchesY + (i / 4) * kSwatchStep;
      if (touch_x >= x && touch_x < x + kSwatchSize && touch_y >= y &&
          touch_y < y + kSwatchSize) {
        color_[0] = kPalette[i][0];
        color_[1] = kPalette[i][1];
        color_[2] = kPalette[i][2];
        color_[3] = 255;
        if (tool_ == kPick) tool_ = kPaint;
      }
    }
  }

  // The canvas: a stroke starts when the stylus touches it.
  s32 canvas_x, canvas_y, canvas_width, canvas_height;
  GetCanvasRect(canvas_x, canvas_y, canvas_width, canvas_height);
  const bool is_on_canvas = is_down && touch_x >= canvas_x &&
                            touch_x < canvas_x + canvas_width &&
                            touch_y >= canvas_y &&
                            touch_y < canvas_y + canvas_height;
  if (is_on_canvas) {
    const s32 x = (touch_x - canvas_x) * static_cast<s32>(target_.width) /
                  canvas_width;
    const s32 y = (touch_y - canvas_y) * static_cast<s32>(target_.height) /
                  canvas_height;
    UseTool(x, y, !was_down_);
  }
  was_down_ = is_on_canvas;

  // The GPU reads the memory: write the cache of the CPU after the changes.
  if (is_dirty_) {
    TextureEditor::Flush(target_);
    canvas_->Flush();
    is_dirty_ = false;
  }
}

void PaintApplication::DrawBottom(sys::Graphics& graphics) {
  const Theme& theme = Theme::GetInstance();
  sys::Graphics::FillScreen(theme.background_color);

  // The texture, over a dark ground that shows the transparent pixels.
  s32 x, y, width, height;
  GetCanvasRect(x, y, width, height);
  sys::Graphics::DrawRect(x, y, width, height, Color(0.15f, 0.15f, 0.15f, 1));
  sys::Graphics::DrawRectWithTexture(x, y, width, height,
                                     Color(1, 1, 1, 1), canvas_);
  sys::Graphics::DrawRectStroke(x - 1, y - 1, width + 2, height + 2, 1,
                                Color(1, 1, 1, 0.4f));

  // The palette, and the current color.
  for (u32 i = 0; i < kPaletteSize; i++) {
    const s32 sx = kPanelX + (i % 4) * kSwatchStep;
    const s32 sy = kSwatchesY + (i / 4) * kSwatchStep;
    sys::Graphics::DrawRect(sx, sy, kSwatchSize, kSwatchSize,
                            Color(kPalette[i][0] / 255.0f,
                                  kPalette[i][1] / 255.0f,
                                  kPalette[i][2] / 255.0f, 1));
  }
  const Color current(color_[0] / 255.0f, color_[1] / 255.0f,
                      color_[2] / 255.0f, 1);
  sys::Graphics::DrawRect(kPanelX, 76, kButtonWidth, 14, current);
  sys::Graphics::DrawRectStroke(kPanelX, 76, kButtonWidth, 14, 1,
                                Color(1, 1, 1, 0.6f));

  // The buttons.
  static const c16* const kToolNames[] = {u"Paint", u"Pick", u"Erase"};
  // After a touch on Save or Del, the button shows the result for a moment.
  const c16* save_label = u"Save";
  const c16* forget_label = u"Del";
  if (message_frames_ != 0) {
    save_label = is_message_ok_ ? u"OK" : u"Error";
    forget_label = is_message_ok_ ? u"OK" : u"None";
  }
  const c16* labels[kButtonCount] = {u"-",     u"+",    kToolNames[tool_],
                                     u"Undo",  u"Reset", save_label,
                                     forget_label, u"Done"};
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  for (u32 i = 0; i < kButtonCount; i++) {
    const Button& button = buttons_[i];
    const bool is_down = button.IsDown();
    Color fill = is_down ? theme.selected_text_color
                         : theme.unselected_text_color;
    fill.a = is_down ? 0.7f : 0.15f;
    sys::Graphics::DrawRect(button.GetX(), button.GetY(), button.GetWidth(),
                            button.GetHeight(), fill);
    const s32 text_width = sys::Graphics::GetTextWidth(labels[i]);
    sys::Graphics::DrawText(button.GetX() +
                                (static_cast<s32>(button.GetWidth()) -
                                 text_width) / 2,
                            button.GetY() + 5, labels[i],
                            theme.unselected_text_color);
  }
  // The size of the brush, above the two buttons of the size.
  sys::Graphics::SetTextScale(0.4f, 0.4f);
  c16 size_text[24];
  core::Utils::Format(size_text, u"Size %d", static_cast<int>(brush_size_));
  sys::Graphics::DrawText(kPanelX, 90, size_text,
                          theme.unselected_text_color);
}

} // namespace ui
