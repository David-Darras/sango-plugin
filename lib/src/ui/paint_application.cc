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
#include "system/native/file.h"
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
constexpr s32 kHalfButtonWidth = 34;
constexpr s32 kButtonHeight = 22;

const u8 kPalette[16][3] = {
    {0, 0, 0},       {255, 255, 255}, {128, 128, 128}, {255, 0, 0},
    {255, 128, 0},   {255, 224, 0},   {128, 255, 0},   {0, 160, 0},
    {0, 224, 224},   {0, 96, 255},    {128, 0, 224},   {255, 0, 192},
    {128, 64, 0},    {255, 160, 192}, {255, 208, 160}, {64, 64, 96},
};

// The tolerance of the bucket and of the replace tool: the sum of the
// differences of the channels.
constexpr s32 kFillTolerance = 40;
constexpr s32 kReplaceTolerance = 90;

// The steps of the color tools.
constexpr f32 kHueStep = 15.0f;
constexpr f32 kSaturationStep = 1.15f;
constexpr f32 kValueStep = 1.12f;

// The TGA file: a header of 18 bytes, then the pixels.
constexpr u32 kTgaHeaderSize = 18;
constexpr u32 kLiveInterval = 45; // The frames between two reads of the file.

// The memory of the copies for the undo and the reset. The memory of the
// device heap stays for all the game: the program allocates it one time, for
// the biggest texture that it meets.
u8* undo_pixels = nullptr;
u8* original_pixels = nullptr;
u32 copy_capacity = 0;

// The memory to read and write the TGA file.
u8* file_buffer = nullptr;
u32 file_buffer_capacity = 0;

// The canvases: one for each size of texture, made when the first texture of
// the size opens.
Texture* canvases[6][6] = {};

// The bucket: the pixels that it already filled, and the pixels to look at.
u32 visited_bits[(renderer::TextureEditor::kMaxSide *
                  renderer::TextureEditor::kMaxSide) / 32];
constexpr u32 kFillStackSize = 4096;
u32 fill_stack[kFillStackSize];

// A button of the right side: the tab where it shows, its position and size.
struct ButtonSpec {
  u8 tab;
  s16 x;
  s16 y;
  s16 width;
};

const ButtonSpec kButtonSpecs[] = {
    // The tab Draw.
    {0, kPanelX, 100, kHalfButtonWidth},             // kSizeDown
    {0, kPanelX + 36, 100, kHalfButtonWidth},        // kSizeUp
    {0, kPanelX, 126, kButtonWidth},                 // kToolButton
    {0, kPanelX, 152, kButtonWidth},                 // kMirror
    {0, kPanelX, 178, kHalfButtonWidth},             // kDrawUndo
    {0, kPanelX + 36, 178, kHalfButtonWidth},        // kDrawReset
    // The tab Color.
    {1, kPanelX, 100, kHalfButtonWidth},             // kHueDown
    {1, kPanelX + 36, 100, kHalfButtonWidth},        // kHueUp
    {1, kPanelX, 126, kHalfButtonWidth},             // kSaturationDown
    {1, kPanelX + 36, 126, kHalfButtonWidth},        // kSaturationUp
    {1, kPanelX, 152, kHalfButtonWidth},             // kValueDown
    {1, kPanelX + 36, 152, kHalfButtonWidth},        // kValueUp
    {1, kPanelX, 178, kHalfButtonWidth},             // kReplace
    {1, kPanelX + 36, 178, kHalfButtonWidth},        // kColorUndo
    // The tab File.
    {2, kPanelX, 100, kHalfButtonWidth},             // kExport
    {2, kPanelX + 36, 100, kHalfButtonWidth},        // kImport
    {2, kPanelX, 126, kButtonWidth},                 // kLive
    {2, kPanelX, 152, kHalfButtonWidth},             // kSave
    {2, kPanelX + 36, 152, kHalfButtonWidth},        // kForget
    {2, kPanelX, 178, kHalfButtonWidth},             // kFileUndo
    {2, kPanelX + 36, 178, kHalfButtonWidth},        // kFileReset
    // All the tabs.
    {3, kPanelX, 210, kHalfButtonWidth},             // kTabButton
    {3, kPanelX + 36, 210, kHalfButtonWidth},        // kDone
};

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

bool EnsureFileBuffer(u32 size) {
  if (file_buffer_capacity >= size) return true;
  u8* buffer = static_cast<u8*>(core::GameFile::AllocateDevice(size, 128));
  if (buffer == nullptr) return false;
  file_buffer = buffer;
  file_buffer_capacity = size;
  return true;
}

u32 HashBytes(const u8* data, u32 size) {
  u32 hash = 2166136261u;
  for (u32 i = 0; i + 4 <= size; i += 4) {
    hash = (hash ^ *reinterpret_cast<const u32*>(data + i)) * 16777619u;
  }
  return hash;
}

void MakeTgaPath(u32 key, c16* out) {
  core::Utils::Format(out, u"sdmc:/sango/textures/export/%08X.tga",
                      static_cast<unsigned>(key));
}

s32 Abs(s32 value) { return value < 0 ? -value : value; }

void RgbToHsv(const u8* rgb, f32& hue, f32& saturation, f32& value) {
  const f32 r = rgb[0] / 255.0f;
  const f32 g = rgb[1] / 255.0f;
  const f32 b = rgb[2] / 255.0f;
  const f32 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
  const f32 min = r < g ? (r < b ? r : b) : (g < b ? g : b);
  const f32 delta = max - min;
  value = max;
  saturation = max > 0.0f ? delta / max : 0.0f;
  if (delta <= 0.0f) {
    hue = 0.0f;
  } else if (max == r) {
    hue = 60.0f * ((g - b) / delta);
  } else if (max == g) {
    hue = 60.0f * ((b - r) / delta + 2.0f);
  } else {
    hue = 60.0f * ((r - g) / delta + 4.0f);
  }
  if (hue < 0.0f) hue += 360.0f;
}

u8 ToByte(f32 value) {
  if (value <= 0.0f) return 0;
  if (value >= 1.0f) return 255;
  return static_cast<u8>(value * 255.0f + 0.5f);
}

void HsvToRgb(f32 hue, f32 saturation, f32 value, u8* rgb) {
  while (hue < 0.0f) hue += 360.0f;
  while (hue >= 360.0f) hue -= 360.0f;
  const f32 chroma = value * saturation;
  const f32 sector = hue / 60.0f;
  const s32 index = static_cast<s32>(sector);
  const f32 fraction = sector - static_cast<f32>(index);
  const f32 p = value - chroma;
  const f32 q = value - chroma * fraction;
  const f32 t = value - chroma * (1.0f - fraction);
  f32 r = value, g = t, b = p;
  switch (index) {
    case 0: r = value; g = t; b = p; break;
    case 1: r = q; g = value; b = p; break;
    case 2: r = p; g = value; b = t; break;
    case 3: r = p; g = q; b = value; break;
    case 4: r = t; g = p; b = value; break;
    default: r = value; g = p; b = q; break;
  }
  rgb[0] = ToByte(r);
  rgb[1] = ToByte(g);
  rgb[2] = ToByte(b);
}

bool IsVisited(u32 x, u32 y, u32 width) {
  const u32 index = y * width + x;
  return (visited_bits[index / 32] >> (index % 32) & 1) != 0;
}

void SetVisited(u32 x, u32 y, u32 width) {
  const u32 index = y * width + x;
  visited_bits[index / 32] |= 1u << (index % 32);
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
  app.is_live_ = false;
  app.has_picked_color_ = false;
  app.tab_ = kTabDraw;
  app.has_copies_ = EnsureCopies(target.GetSize());
  if (app.has_copies_) {
    memcpy(original_pixels, target.pixels, target.GetSize());
    memcpy(undo_pixels, target.pixels, target.GetSize());
  }
  app.RedrawCanvas();
  app.original_hash_ = TextureEditor::GetOriginalHash(target);
  app.is_saved_ = TextureEditor::IsSaved(target, app.original_hash_);
  app.message_frames_ = 0;

  for (u32 i = 0; i < kButtonCount; i++) {
    app.buttons_[i].Initialize(kButtonSpecs[i].x, kButtonSpecs[i].y,
                               kButtonSpecs[i].width, kButtonHeight);
    app.buttons_[i].Reset();
  }

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

void PaintApplication::StampOne(s32 x, s32 y) {
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

void PaintApplication::Stamp(s32 x, s32 y) {
  StampOne(x, y);
  if (is_mirror_) {
    const s32 mirror_x = static_cast<s32>(target_.width) - 1 - x;
    if (mirror_x != x) StampOne(mirror_x, y);
  }
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

void PaintApplication::Fill(s32 start_x, s32 start_y) {
  const s32 width = static_cast<s32>(target_.width);
  const s32 height = static_cast<s32>(target_.height);
  if (start_x < 0 || start_y < 0 || start_x >= width || start_y >= height) {
    return;
  }
  u8 target_color[4];
  TextureEditor::ReadPixel(target_, start_x, start_y, target_color);
  u8 rgba[4] = {color_[0], color_[1], color_[2], color_[3]};
  // The pixels that look like the first pixel are in the area.
  auto matches = [&](s32 x, s32 y) {
    u8 pixel[4];
    TextureEditor::ReadPixel(target_, x, y, pixel);
    return Abs(pixel[0] - target_color[0]) + Abs(pixel[1] - target_color[1]) +
               Abs(pixel[2] - target_color[2]) +
               Abs(pixel[3] - target_color[3]) <=
           kFillTolerance;
  };

  memset(visited_bits, 0, (target_.width * target_.height + 7) / 8);
  u32 stack_size = 0;
  fill_stack[stack_size++] = (static_cast<u32>(start_y) << 16) | start_x;
  while (stack_size != 0) {
    const u32 position = fill_stack[--stack_size];
    const s32 x = position & 0xFFFF;
    const s32 y = position >> 16;
    if (IsVisited(x, y, width) || !matches(x, y)) continue;
    // The span of the row that has the same color.
    s32 left = x;
    while (left > 0 && !IsVisited(left - 1, y, width) && matches(left - 1, y)) {
      left--;
    }
    s32 right = x;
    while (right < width - 1 && !IsVisited(right + 1, y, width) &&
           matches(right + 1, y)) {
      right++;
    }
    for (s32 i = left; i <= right; i++) {
      SetVisited(i, y, width);
      TextureEditor::WritePixel(target_, i, y, rgba);
      CopyPixelToCanvas(i, y);
    }
    // The rows above and below: one seed for each part of the span.
    for (s32 row = y - 1; row <= y + 1; row += 2) {
      if (row < 0 || row >= height) continue;
      bool is_inside = false;
      for (s32 i = left; i <= right; i++) {
        const bool is_match = !IsVisited(i, row, width) && matches(i, row);
        if (is_match && !is_inside && stack_size < kFillStackSize) {
          fill_stack[stack_size++] = (static_cast<u32>(row) << 16) | i;
        }
        is_inside = is_match;
      }
    }
  }
  is_dirty_ = true;
}

void PaintApplication::AdjustColors(f32 hue, f32 saturation, f32 value) {
  for (u32 y = 0; y < target_.height; y++) {
    for (u32 x = 0; x < target_.width; x++) {
      u8 rgba[4];
      TextureEditor::ReadPixel(target_, x, y, rgba);
      f32 h, s, v;
      RgbToHsv(rgba, h, s, v);
      h += hue;
      s *= saturation;
      v *= value;
      if (s > 1.0f) s = 1.0f;
      if (v > 1.0f) v = 1.0f;
      HsvToRgb(h, s, v, rgba);
      TextureEditor::WritePixel(target_, x, y, rgba);
    }
  }
  RedrawCanvas();
  is_dirty_ = true;
}

void PaintApplication::ReplacePickedColor() {
  const s32 picked_value = picked_color_[0] > picked_color_[1]
                               ? (picked_color_[0] > picked_color_[2]
                                      ? picked_color_[0]
                                      : picked_color_[2])
                               : (picked_color_[1] > picked_color_[2]
                                      ? picked_color_[1]
                                      : picked_color_[2]);
  const f32 divisor = picked_value > 1 ? static_cast<f32>(picked_value) : 1.0f;
  for (u32 y = 0; y < target_.height; y++) {
    for (u32 x = 0; x < target_.width; x++) {
      u8 rgba[4];
      TextureEditor::ReadPixel(target_, x, y, rgba);
      if (Abs(rgba[0] - picked_color_[0]) + Abs(rgba[1] - picked_color_[1]) +
              Abs(rgba[2] - picked_color_[2]) >
          kReplaceTolerance) {
        continue;
      }
      // The new color keeps the shade of the old pixel.
      const u8 value = rgba[0] > rgba[1] ? (rgba[0] > rgba[2] ? rgba[0] : rgba[2])
                                         : (rgba[1] > rgba[2] ? rgba[1] : rgba[2]);
      const f32 ratio = static_cast<f32>(value) / divisor;
      for (u32 i = 0; i < 3; i++) {
        const f32 shaded = color_[i] * ratio;
        rgba[i] = shaded >= 255.0f ? 255 : static_cast<u8>(shaded);
      }
      TextureEditor::WritePixel(target_, x, y, rgba);
    }
  }
  RedrawCanvas();
  is_dirty_ = true;
}

void PaintApplication::SaveUndo() {
  if (has_copies_) memcpy(undo_pixels, target_.pixels, target_.GetSize());
}

void PaintApplication::RestoreUndo() {
  if (!has_copies_) return;
  memcpy(target_.pixels, undo_pixels, target_.GetSize());
  RedrawCanvas();
  is_dirty_ = true;
}

void PaintApplication::RestoreOriginal() {
  if (!has_copies_) return;
  memcpy(target_.pixels, original_pixels, target_.GetSize());
  RedrawCanvas();
  is_dirty_ = true;
}

bool PaintApplication::ExportFile() {
  const u32 size = kTgaHeaderSize + target_.width * target_.height * 4;
  if (!EnsureFileBuffer(size)) return false;
  u8* data = file_buffer;
  memset(data, 0, kTgaHeaderSize);
  data[2] = 2; // Uncompressed, true colors.
  data[12] = static_cast<u8>(target_.width);
  data[13] = static_cast<u8>(target_.width >> 8);
  data[14] = static_cast<u8>(target_.height);
  data[15] = static_cast<u8>(target_.height >> 8);
  data[16] = 32; // Bits for each pixel.
  data[17] = 0x28; // 8 bits of alpha, and the first row is the top row.
  u8* out = data + kTgaHeaderSize;
  for (u32 y = 0; y < target_.height; y++) {
    for (u32 x = 0; x < target_.width; x++) {
      u8 rgba[4];
      TextureEditor::ReadPixel(target_, x, y, rgba);
      out[0] = rgba[2];
      out[1] = rgba[1];
      out[2] = rgba[0];
      out[3] = rgba[3];
      out += 4;
    }
  }
  sys::File::CreateDirectory(u"sdmc:/sango");
  sys::File::CreateDirectory(u"sdmc:/sango/textures");
  sys::File::CreateDirectory(u"sdmc:/sango/textures/export");
  c16 path[72];
  MakeTgaPath(TextureEditor::GetFileKey(target_, original_hash_), path);
  {
    sys::File file(path, true);
    if (!file.IsOpen()) return false;
    file.Write(data, size);
  }
  live_hash_ = HashBytes(data, size);
  return true;
}

bool PaintApplication::ImportBuffer(const u8* data, u32 size) {
  if (size < kTgaHeaderSize) return false;
  const u32 id_length = data[0];
  const u32 width = data[12] | (data[13] << 8);
  const u32 height = data[14] | (data[15] << 8);
  const u32 bits = data[16];
  const bool is_top_first = (data[17] & 0x20) != 0;
  if (data[1] != 0 || data[2] != 2 || (bits != 24 && bits != 32) ||
      width != target_.width || height != target_.height) {
    return false;
  }
  const u32 bytes = bits / 8;
  const u32 start = kTgaHeaderSize + id_length;
  if (size < start + width * height * bytes) return false;

  SaveUndo();
  for (u32 y = 0; y < height; y++) {
    const u32 row = is_top_first ? y : height - 1 - y;
    const u8* in = data + start + row * width * bytes;
    for (u32 x = 0; x < width; x++) {
      const u8 rgba[4] = {in[2], in[1], in[0],
                          static_cast<u8>(bytes == 4 ? in[3] : 255)};
      TextureEditor::WritePixel(target_, x, y, rgba);
      in += bytes;
    }
  }
  RedrawCanvas();
  is_dirty_ = true;
  return true;
}

bool PaintApplication::ImportFile() {
  const u32 capacity = kTgaHeaderSize + target_.width * target_.height * 4 + 64;
  if (!EnsureFileBuffer(capacity)) return false;
  c16 path[72];
  MakeTgaPath(TextureEditor::GetFileKey(target_, original_hash_), path);
  const u32 size = sys::File::ReadAll(path, file_buffer, file_buffer_capacity);
  if (size == 0) return false;
  live_hash_ = HashBytes(file_buffer, size);
  return ImportBuffer(file_buffer, size);
}

void PaintApplication::PollLiveFile() {
  const u32 capacity = kTgaHeaderSize + target_.width * target_.height * 4 + 64;
  if (!EnsureFileBuffer(capacity)) return;
  c16 path[72];
  MakeTgaPath(TextureEditor::GetFileKey(target_, original_hash_), path);
  const u32 size = sys::File::ReadAll(path, file_buffer, file_buffer_capacity);
  if (size == 0) return;
  const u32 hash = HashBytes(file_buffer, size);
  if (hash == live_hash_) return;
  live_hash_ = hash;
  // A file that the program is writing, or a wrong file, waits for the next
  // read.
  if (ImportBuffer(file_buffer, size)) SetMessage(u"Reloaded");
}

void PaintApplication::UseTool(s32 x, s32 y, bool is_start) {
  if (tool_ == kPick) {
    if (x < 0 || y < 0 || x >= static_cast<s32>(target_.width) ||
        y >= static_cast<s32>(target_.height)) {
      return;
    }
    u8 rgba[4];
    TextureEditor::ReadPixel(target_, x, y, rgba);
    color_[0] = picked_color_[0] = rgba[0];
    color_[1] = picked_color_[1] = rgba[1];
    color_[2] = picked_color_[2] = rgba[2];
    color_[3] = 255;
    has_picked_color_ = true;
    return;
  }
  if (tool_ == kFill) {
    // The bucket works one time for each touch.
    if (!is_start) return;
    SaveUndo();
    Fill(x, y);
    if (is_mirror_) Fill(static_cast<s32>(target_.width) - 1 - x, y);
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

void PaintApplication::SetMessage(const c16* text) {
  message_ = text;
  message_frames_ = 90;
}

void PaintApplication::SelectTab(Tab tab) {
  tab_ = tab;
  for (Button& button : buttons_) button.Reset();
}

bool PaintApplication::IsButtonActive(ButtonId id) const {
  const u8 tab = kButtonSpecs[id].tab;
  return tab == kAllTabs || tab == tab_;
}

bool PaintApplication::IsPressed(ButtonId id) const {
  return IsButtonActive(id) && buttons_[id].IsReleased();
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
  for (u32 i = 0; i < kButtonCount; i++) {
    if (IsButtonActive(static_cast<ButtonId>(i))) buttons_[i].Update();
  }

  // The buttons of all the tabs.
  if (IsPressed(kDone)) {
    Close();
    return;
  }
  if (IsPressed(kTabButton)) {
    SelectTab(static_cast<Tab>((tab_ + 1) % kTabCount));
    return;
  }

  // The tab Draw.
  if (IsPressed(kSizeDown) && brush_size_ > 1) brush_size_--;
  if (IsPressed(kSizeUp) && brush_size_ < kMaxBrush) brush_size_++;
  if (IsPressed(kToolButton)) {
    do {
      tool_ = static_cast<Tool>((tool_ + 1) % kToolCount);
    } while (tool_ == kErase && !TextureEditor::HasAlpha(target_.format));
  }
  if (IsPressed(kMirror)) is_mirror_ = !is_mirror_;
  if (IsPressed(kDrawUndo) || IsPressed(kColorUndo) || IsPressed(kFileUndo)) {
    RestoreUndo();
  }
  if (IsPressed(kDrawReset) || IsPressed(kFileReset)) RestoreOriginal();

  // The tab Color.
  if (IsPressed(kHueDown)) SaveUndo(), AdjustColors(-kHueStep, 1.0f, 1.0f);
  if (IsPressed(kHueUp)) SaveUndo(), AdjustColors(kHueStep, 1.0f, 1.0f);
  if (IsPressed(kSaturationDown)) {
    SaveUndo();
    AdjustColors(0.0f, 1.0f / kSaturationStep, 1.0f);
  }
  if (IsPressed(kSaturationUp)) {
    SaveUndo();
    AdjustColors(0.0f, kSaturationStep, 1.0f);
  }
  if (IsPressed(kValueDown)) {
    SaveUndo();
    AdjustColors(0.0f, 1.0f, 1.0f / kValueStep);
  }
  if (IsPressed(kValueUp)) {
    SaveUndo();
    AdjustColors(0.0f, 1.0f, kValueStep);
  }
  if (IsPressed(kReplace)) {
    if (has_picked_color_) {
      SaveUndo();
      ReplacePickedColor();
    } else {
      SetMessage(u"Pick a color");
    }
  }

  // The tab File.
  if (IsPressed(kExport)) SetMessage(ExportFile() ? u"Exported" : u"Error");
  if (IsPressed(kImport)) SetMessage(ImportFile() ? u"Imported" : u"No file");
  if (IsPressed(kLive)) {
    is_live_ = !is_live_;
    live_frames_ = 0;
    // The file starts as the texture: the player opens it on a computer.
    if (is_live_) SetMessage(ExportFile() ? u"Live" : u"Error");
  }
  if (IsPressed(kSave)) {
    is_saved_ = TextureEditor::Save(target_, original_hash_) || is_saved_;
    SetMessage(is_saved_ ? u"Saved" : u"Error");
  }
  if (IsPressed(kForget)) {
    SetMessage(TextureEditor::Forget(target_, original_hash_) ? u"Deleted"
                                                              : u"None");
    is_saved_ = false;
  }

  if (message_frames_ != 0 && --message_frames_ == 0) message_ = nullptr;
  if (is_live_ && ++live_frames_ >= kLiveInterval) {
    live_frames_ = 0;
    PollLiveFile();
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

  // The labels of the buttons.
  static const c16* const kToolNames[] = {u"Paint", u"Pick", u"Erase", u"Fill"};
  static const c16* const kTabNames[] = {u"Draw", u"Color", u"File"};
  const c16* labels[kButtonCount] = {
      u"-",      u"+",        kToolNames[tool_],
      is_mirror_ ? u"Mirror: On" : u"Mirror: Off",
      u"Undo",   u"Reset",
      u"Hue-",   u"Hue+",     u"Sat-",    u"Sat+",   u"Val-",   u"Val+",
      u"Replace", u"Undo",
      u"Export", u"Import",   is_live_ ? u"Live: On" : u"Live: Off",
      u"Save",   u"Del",      u"Undo",    u"Reset",
      kTabNames[tab_], u"Done"};
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  for (u32 i = 0; i < kButtonCount; i++) {
    if (!IsButtonActive(static_cast<ButtonId>(i))) continue;
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

  // The line above the buttons: a message, or the size of the brush.
  sys::Graphics::SetTextScale(0.4f, 0.4f);
  if (message_ != nullptr) {
    sys::Graphics::DrawText(kPanelX, 90, message_, theme.selected_text_color);
  } else {
    c16 line[24];
    core::Utils::Format(line, u"Size %d", static_cast<int>(brush_size_));
    sys::Graphics::DrawText(kPanelX, 90, line, theme.unselected_text_color);
  }
}

} // namespace ui
