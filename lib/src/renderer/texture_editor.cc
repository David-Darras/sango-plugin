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
 * @file texture_editor.cc
 * @brief Reads and writes the pixels of the textures of the 3D models.
 *
 * The declarations are in renderer/patch/texture_editor.h.
 */

#include "renderer/patch/texture_editor.h"

#include <3ds.h>

#include "core/utils.h"
#include "renderer/address.h"
#include "renderer/constant/texture_format.h"
#include "renderer/patch/model_viewer.h"
#include "system/native/file.h"

namespace renderer {

namespace {
// The texture that the game makes while it runs: the layout of its object.
constexpr u32 kAddressOffset = 4; // The address of the pixels, for the GPU.
constexpr u32 kSizeOffset = 8; // The width in the high half, the height in the low half.
constexpr u32 kFormatOffset = 0xC;
constexpr u32 kMipmapOffset = 0xD;
// The kind that ModelViewer::ReadTextureUnit() gives for such a texture.
constexpr u8 kRuntimeTextureKind = 3;

// Returns the address of GPU memory for the CPU, or 0. The GPU reads the
// physical addresses: VRAM and the linear memory (FCRAM).
u8* ToCpuAddress(u32 physical) {
  if (physical >= 0x18000000 && physical < 0x18600000) {
    return reinterpret_cast<u8*>(physical + 0x07000000);
  }
  if (physical >= 0x20000000 && physical < 0x28000000) {
    return reinterpret_cast<u8*>(physical - 0x0C000000);
  }
  return nullptr;
}

bool IsPowerOfTwo(u32 value) { return value != 0 && (value & (value - 1)) == 0; }

// The position of a pixel: the bits of x and y alternate inside a tile of 8 x 8.
u32 GetPixelIndex(const TextureTarget& target, u32 x, u32 y) {
  // The rows start from the bottom of the image.
  const u32 row = target.height - 1 - y;
  const u32 in_tile = (x & 1) | ((row & 1) << 1) | ((x & 2) << 1) |
                      ((row & 2) << 2) | ((x & 4) << 2) | ((row & 4) << 3);
  const u32 tile = (row / 8) * (target.width / 8) + x / 8;
  return tile * 64 + in_tile;
}

// The saved textures: a list of keys (in the file index.bin) tells which
// textures have a file, so that the plugin reads the SD card only for them.
const c16* const kFolder = u"sdmc:/sango/textures";
const c16* const kIndexPath = u"sdmc:/sango/textures/index.bin";
constexpr u32 kMaxSaved = 256;
u32 saved_keys[kMaxSaved];
u32 saved_count = 0;

// The header of a file of a saved texture.
struct SavedHeader {
  u32 signature; // "STX1"
  u32 width;
  u32 height;
  u32 format;
  u32 original_hash;
  u32 size;
};
constexpr u32 kSignature = 0x31585453;

// The textures that the editor changed with a saved file: the pixels and the
// hash of the original pixels.
struct Applied {
  const u8* pixels;
  u32 original_hash;
};
constexpr u32 kMaxApplied = 32;
Applied applied[kMaxApplied];
u32 applied_next = 0;

// The new models wait here: the game writes their pixels a little after it
// puts the model in the scene.
struct Pending {
  const H3dModel* model;
  u32 frames;
};
constexpr u32 kMaxPending = 8;
constexpr u32 kPendingFrames = 90;
constexpr u32 kPendingStep = 6;
Pending pending[kMaxPending];

u32 HashBytes(const u8* data, u32 size) {
  u32 hash = 2166136261u;
  const u32* words = reinterpret_cast<const u32*>(data);
  for (u32 i = 0; i < size / 4; i++) hash = (hash ^ words[i]) * 16777619u;
  return hash;
}

u32 MakeKey(const TextureTarget& target, u32 hash) {
  return hash ^ (target.width * 73856093u) ^ (target.height * 19349663u) ^
         (target.format * 83492791u);
}

void MakePath(u32 key, c16* out) {
  core::Utils::Format(out, u"sdmc:/sango/textures/%08X.tex",
                      static_cast<unsigned>(key));
}

bool HasKey(u32 key) {
  for (u32 i = 0; i < saved_count; i++) {
    if (saved_keys[i] == key) return true;
  }
  return false;
}

void SaveIndex() {
  sys::File::CreateDirectory(u"sdmc:/sango");
  sys::File::CreateDirectory(kFolder);
  sys::File file(kIndexPath, true);
  if (file.IsOpen() && saved_count != 0) {
    file.Write(saved_keys, saved_count * sizeof(u32));
  }
}

void RememberApplied(const u8* pixels, u32 original_hash) {
  for (Applied& entry : applied) {
    if (entry.pixels == pixels) {
      entry.original_hash = original_hash;
      return;
    }
  }
  applied[applied_next].pixels = pixels;
  applied[applied_next].original_hash = original_hash;
  applied_next = (applied_next + 1) % kMaxApplied;
}

// Gives the saved pixels to a texture, when the pixels are the original ones
// of a saved file. Returns true when it changed the texture.
bool ApplyToTexture(const TextureTarget& target) {
  const u32 original_hash =
      HashBytes(target.pixels, target.GetSize());
  const u32 key = MakeKey(target, original_hash);
  if (!HasKey(key)) return false;
  c16 path[64];
  MakePath(key, path);
  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) return false;
  SavedHeader header = {};
  file.Read(&header, sizeof(header));
  if (header.signature != kSignature || header.width != target.width ||
      header.height != target.height || header.format != target.format ||
      header.size != target.GetSize()) {
    return false;
  }
  file.Read(target.pixels, header.size);
  TextureEditor::Flush(target);
  RememberApplied(target.pixels, original_hash);
  return true;
}

u8 Expand(u32 value, u32 bits) {
  switch (bits) {
    case 4: return static_cast<u8>(value * 17);
    case 5: return static_cast<u8>((value << 3) | (value >> 2));
    default: return static_cast<u8>((value << 2) | (value >> 4)); // 6 bits.
  }
}
} // namespace

u32 TextureTarget::GetBytesPerPixel() const {
  switch (static_cast<TextureFormat>(format)) {
    case TextureFormat::kRgba8: return 4;
    case TextureFormat::kRgb8: return 3;
    case TextureFormat::kL8: return 1;
    default: return 2;
  }
}

bool TextureEditor::IsFormatSupported(u8 format) {
  switch (static_cast<TextureFormat>(format)) {
    case TextureFormat::kRgba8:
    case TextureFormat::kRgb8:
    case TextureFormat::kRgba5551:
    case TextureFormat::kRgb565:
    case TextureFormat::kRgba4:
    case TextureFormat::kL8:
      return true;
    default:
      return false;
  }
}

bool TextureEditor::HasAlpha(u8 format) {
  switch (static_cast<TextureFormat>(format)) {
    case TextureFormat::kRgba8:
    case TextureFormat::kRgba5551:
    case TextureFormat::kRgba4:
      return true;
    default:
      return false;
  }
}

const c8* TextureEditor::GetFormatName(u8 format) {
  static const c8* const kNames[] = {"RGBA8", "RGB8", "RGBA5551", "RGB565",
                                     "RGBA4", "LA8",  "HILO8",    "L8",
                                     "A8",    "LA4",  "L4",       "A4",
                                     "ETC1",  "ETC1A4"};
  return format < sizeof(kNames) / sizeof(kNames[0]) ? kNames[format]
                                                       : "Unknown";
}

bool TextureEditor::Locate(const H3dModel* model, u32 material, u32 unit,
                           TextureTarget& out, const c8** error) {
  out = TextureTarget();
#ifdef GAME_ORAS
  TextureUnitInfo info;
  if (!ModelViewer::ReadTextureUnit(model, material, unit, info) ||
      info.kind != kRuntimeTextureKind ||
      info.words[0] != address::kH3dDynamicTextureVtable) {
    *error = "This texture is not one that the game makes while it runs.";
    return false;
  }
  const u32 size = info.words[kSizeOffset / 4];
  out.model = model;
  out.material = material;
  out.unit = unit;
  out.object = info.object;
  out.width = size >> 16;
  out.height = size & 0xFFFF;
  out.format = static_cast<u8>(info.words[kFormatOffset / 4] & 0xFF);
  out.mipmap_count = static_cast<u8>((info.words[kFormatOffset / 4] >> 8) & 0xFF);
  out.pixels = ToCpuAddress(info.words[kAddressOffset / 4]);
  if (out.pixels == nullptr) {
    *error = "The pixels are not in a memory that the plugin knows.";
    return false;
  }
  if (!IsPowerOfTwo(out.width) || !IsPowerOfTwo(out.height) ||
      out.width < 8 || out.height < 8 || out.width > kMaxSide ||
      out.height > kMaxSide) {
    *error = "The size of the texture is not supported (8 to 256).";
    return false;
  }
  if (!IsFormatSupported(out.format)) {
    *error = "The format of the texture is not supported (ETC1, LA8...).";
    return false;
  }
  return true;
#else
  (void)model;
  (void)material;
  (void)unit;
  *error = "The texture editor needs the data of Alpha Sapphire.";
  return false;
#endif
}

bool TextureEditor::IsValid(const TextureTarget& target) {
  if (!ModelViewer::IsKnown(target.model)) return false;
  TextureUnitInfo info;
  if (!ModelViewer::ReadTextureUnit(target.model, target.material, target.unit,
                                    info)) {
    return false;
  }
  return info.kind == kRuntimeTextureKind && info.object == target.object &&
         ToCpuAddress(info.words[kAddressOffset / 4]) == target.pixels;
}

void TextureEditor::ReadPixel(const TextureTarget& target, u32 x, u32 y,
                              u8* rgba) {
  const u32 bytes = target.GetBytesPerPixel();
  const u8* data = target.pixels + GetPixelIndex(target, x, y) * bytes;
  rgba[3] = 255;
  switch (static_cast<TextureFormat>(target.format)) {
    case TextureFormat::kRgba8:
      rgba[3] = data[0];
      rgba[2] = data[1];
      rgba[1] = data[2];
      rgba[0] = data[3];
      break;
    case TextureFormat::kRgb8:
      rgba[2] = data[0];
      rgba[1] = data[1];
      rgba[0] = data[2];
      break;
    case TextureFormat::kRgba5551: {
      const u32 v = data[0] | (data[1] << 8);
      rgba[0] = Expand((v >> 11) & 31, 5);
      rgba[1] = Expand((v >> 6) & 31, 5);
      rgba[2] = Expand((v >> 1) & 31, 5);
      rgba[3] = (v & 1) ? 255 : 0;
      break;
    }
    case TextureFormat::kRgb565: {
      const u32 v = data[0] | (data[1] << 8);
      rgba[0] = Expand((v >> 11) & 31, 5);
      rgba[1] = Expand((v >> 5) & 63, 6);
      rgba[2] = Expand(v & 31, 5);
      break;
    }
    case TextureFormat::kRgba4: {
      const u32 v = data[0] | (data[1] << 8);
      rgba[0] = Expand((v >> 12) & 15, 4);
      rgba[1] = Expand((v >> 8) & 15, 4);
      rgba[2] = Expand((v >> 4) & 15, 4);
      rgba[3] = Expand(v & 15, 4);
      break;
    }
    default: // L8
      rgba[0] = rgba[1] = rgba[2] = data[0];
      break;
  }
}

void TextureEditor::WritePixel(const TextureTarget& target, u32 x, u32 y,
                               const u8* rgba) {
  const u32 bytes = target.GetBytesPerPixel();
  u8* data = target.pixels + GetPixelIndex(target, x, y) * bytes;
  const u32 r = rgba[0];
  const u32 g = rgba[1];
  const u32 b = rgba[2];
  const u32 a = rgba[3];
  switch (static_cast<TextureFormat>(target.format)) {
    case TextureFormat::kRgba8:
      data[0] = static_cast<u8>(a);
      data[1] = static_cast<u8>(b);
      data[2] = static_cast<u8>(g);
      data[3] = static_cast<u8>(r);
      break;
    case TextureFormat::kRgb8:
      data[0] = static_cast<u8>(b);
      data[1] = static_cast<u8>(g);
      data[2] = static_cast<u8>(r);
      break;
    case TextureFormat::kRgba5551: {
      const u32 v = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) |
                    (a >= 128 ? 1 : 0);
      data[0] = static_cast<u8>(v);
      data[1] = static_cast<u8>(v >> 8);
      break;
    }
    case TextureFormat::kRgb565: {
      const u32 v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      data[0] = static_cast<u8>(v);
      data[1] = static_cast<u8>(v >> 8);
      break;
    }
    case TextureFormat::kRgba4: {
      const u32 v = ((r >> 4) << 12) | ((g >> 4) << 8) | ((b >> 4) << 4) |
                    (a >> 4);
      data[0] = static_cast<u8>(v);
      data[1] = static_cast<u8>(v >> 8);
      break;
    }
    default: // L8
      data[0] = static_cast<u8>((r * 77 + g * 150 + b * 29) >> 8);
      break;
  }
}

void TextureEditor::Initialize() {
  saved_count = 0;
  sys::File file;
  file.Open(kIndexPath, sys::File::kRead);
  if (!file.IsOpen()) return;
  const s32 read = file.Read(saved_keys, sizeof(saved_keys));
  if (read > 0) saved_count = static_cast<u32>(read) / sizeof(u32);
  if (saved_count > kMaxSaved) saved_count = kMaxSaved;
}

void TextureEditor::OnModelAdded(const H3dModel* model) {
#ifdef GAME_ORAS
  // The game made new textures for this model: the hashes of the old ones are
  // not valid for the new pixels.
  const u32 materials = ModelViewer::GetMaterialCount(model);
  for (u32 material = 0; material < materials; material++) {
    for (u32 unit = 0; unit < 3; unit++) {
      TextureTarget target;
      const c8* error = nullptr;
      if (!Locate(model, material, unit, target, &error)) continue;
      for (Applied& entry : applied) {
        if (entry.pixels == target.pixels) entry.pixels = nullptr;
      }
    }
  }
  if (saved_count == 0) return;
  for (Pending& entry : pending) {
    if (entry.model == model) {
      entry.frames = kPendingFrames;
      return;
    }
  }
  for (Pending& entry : pending) {
    if (entry.frames == 0) {
      entry.model = model;
      entry.frames = kPendingFrames;
      return;
    }
  }
#else
  (void)model;
#endif
}

void TextureEditor::Update() {
#ifdef GAME_ORAS
  for (Pending& entry : pending) {
    if (entry.frames == 0) continue;
    entry.frames--;
    if (!ModelViewer::IsKnown(entry.model)) {
      entry.frames = 0;
      continue;
    }
    if (entry.frames % kPendingStep != 0) continue;
    const u32 materials = ModelViewer::GetMaterialCount(entry.model);
    for (u32 material = 0; material < materials; material++) {
      for (u32 unit = 0; unit < 3; unit++) {
        TextureTarget target;
        const c8* error = nullptr;
        if (Locate(entry.model, material, unit, target, &error)) {
          ApplyToTexture(target);
        }
      }
    }
  }
#endif
}

u32 TextureEditor::GetOriginalHash(const TextureTarget& target) {
  for (const Applied& entry : applied) {
    if (entry.pixels == target.pixels) return entry.original_hash;
  }
  // The first look at this texture: its pixels are the original ones. The
  // editor keeps the hash, because the player changes the pixels after.
  const u32 hash = HashBytes(target.pixels, target.GetSize());
  RememberApplied(target.pixels, hash);
  return hash;
}

bool TextureEditor::IsSaved(const TextureTarget& target, u32 original_hash) {
  return HasKey(MakeKey(target, original_hash));
}

bool TextureEditor::Save(const TextureTarget& target, u32 original_hash) {
  if (saved_count >= kMaxSaved && !IsSaved(target, original_hash)) return false;
  sys::File::CreateDirectory(u"sdmc:/sango");
  sys::File::CreateDirectory(kFolder);
  const u32 key = MakeKey(target, original_hash);
  c16 path[64];
  MakePath(key, path);
  {
    sys::File file(path, true);
    if (!file.IsOpen()) return false;
    SavedHeader header;
    header.signature = kSignature;
    header.width = target.width;
    header.height = target.height;
    header.format = target.format;
    header.original_hash = original_hash;
    header.size = target.GetSize();
    file.Write(&header, sizeof(header));
    file.Write(target.pixels, header.size);
  }
  if (!HasKey(key)) {
    saved_keys[saved_count++] = key;
    SaveIndex();
  }
  RememberApplied(target.pixels, original_hash);
  return true;
}

bool TextureEditor::Forget(const TextureTarget& target, u32 original_hash) {
  const u32 key = MakeKey(target, original_hash);
  if (!HasKey(key)) return false;
  c16 path[64];
  MakePath(key, path);
  sys::File::Delete(path);
  for (u32 i = 0; i < saved_count; i++) {
    if (saved_keys[i] == key) {
      saved_keys[i] = saved_keys[--saved_count];
      break;
    }
  }
  SaveIndex();
  return true;
}

void TextureEditor::Flush(const TextureTarget& target) {
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE,
                           reinterpret_cast<u32>(target.pixels),
                           target.GetSize());
}

} // namespace renderer
