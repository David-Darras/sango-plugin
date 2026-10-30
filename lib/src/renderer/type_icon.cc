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
 * @file type_icon.cc
 * @brief The icons of the new types in the layouts of the game.
 *
 * The declarations are in renderer/patch/type_icon.h. The types are the ones
 * that battle::GameExtension::AddType() registered.
 */

#include "renderer/patch/type_icon.h"

#include <3ds.h>
#include <cstring>

#include "battle/address.h"
#include "battle/patch/game_extension.h"
#include "core/game_file.h"
#include "core/hook.h"
#include "pokemon/constant/type.h"
#include "renderer/address.h"
#include "system/native/file.h"
#include "ui/log.h"

namespace renderer {
namespace {
// The texture of a layout, as the game gives it: the object of the GPU, the
// address of the pixels, the size of the image, the size of the texture and
// the format.
struct LayoutTexture {
  u32 object;
  u32 physical_address;
  u16 width;
  u16 height;
  u16 real_width;
  u16 real_height;
  u8 format;
  u8 padding[3];
};
static_assert(sizeof(LayoutTexture) == 20,
              "LayoutTexture must have the layout of the game");

// In the resource of the common graphic font: the image of the type Normal,
// and the number of types that the game has. The next image is for the type
// that has the number 18, and so on.
constexpr u32 kFirstIcon = 24;
constexpr u32 kGameTypeCount = 18;
constexpr u32 kMaxTypes = battle::GameExtension::kMaxTypes;
constexpr u16 kIconWidth = 50;
constexpr u16 kIconHeight = 20;
// The function that makes the texture reads the memory of the image in the
// footer of the BCLIM file when it gets 0: our file asks for the main memory.
constexpr u32 kMemoryFromFile = 0;
constexpr u32 kFooterSize = 0x28;
constexpr u32 kFooterSignature = 0x4D494C43; // 'CLIM'
constexpr u32 kFooterFileSizeOffset = 0x0C;
constexpr u32 kAlignment = 128;
constexpr u32 kMaxFileSize = 16 * 1024;

// The first parameter is the place of the returned texture.
core::Hook<void(void*, const void*, u32)> get_image_hook;
core::Hook<void(void*, u32, f32)> seek_animation_hook;
core::Hook<void(void*, u32, void*)> play_material_animation_hook;
core::Hook<void(void*, void*, u32, u32)> show_type_icon_hook;

// The last frame that a layout got: the game sets the frame of the colour just
// before it sets the icon of the type.
struct LastFrame {
  void* layout;
  u32 animation;
  f32 frame;
};
LastFrame last_frame = {nullptr, 0, -1.0f};

// The texture of each type that has an image. One try for each type: a missing
// file stays missing.
enum class State : u8 { kNotTried, kReady, kFailed };
LayoutTexture textures[kMaxTypes];
State states[kMaxTypes];

// The type icons are 50 x 20 images. The object of the GPU can be 0 for a
// valid texture, and the format does not matter here.
bool IsTypeIcon(const LayoutTexture& texture) {
  return texture.physical_address != 0 && texture.width == kIconWidth &&
         texture.height == kIconHeight;
}

// The type that has an image to give, or null.
const battle::TypeSpec* FindImageType(u32 type) {
  if (type >= kMaxTypes) return nullptr;
  const battle::TypeSpec* spec = battle::GameExtension::FindType(type);
  return spec != nullptr && spec->name != nullptr ? spec : nullptr;
}

// Reads the image of a type from the SD card, and asks the game to make a
// texture of it. Returns the texture, or null.
const LayoutTexture* LoadTypeImage(u32 type) {
  if (states[type] == State::kReady) return &textures[type];
  if (states[type] == State::kFailed) return nullptr;
  states[type] = State::kFailed;
  const battle::TypeSpec* spec = FindImageType(type);
  if (spec == nullptr) return nullptr;

  c16 path[80];
  if (!battle::GameExtension::GetTypeIconPath(*spec, path, SIZE(path))) {
    return nullptr;
  }
  static u8 file[kMaxFileSize];
  const u32 size = TypeIcon::ReadFile(path, file, sizeof(file));
  if (size == 0) {
    // A type of the game keeps its icon when its file is missing.
    if (type < kGameTypeCount) return nullptr;
    ui::Log::Print(u"type icon %u: no image in the file", type);
    return nullptr;
  }

  // The GPU reads the image: it needs a memory that stays, with an alignment.
  auto* image =
      static_cast<u8*>(core::GameFile::AllocateDevice(size, kAlignment));
  if (image == nullptr) {
    ui::Log::Print(u"type icon %u: no GPU memory for the image", type);
    return nullptr;
  }
  memcpy(image, file, size);
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, reinterpret_cast<u32>(image),
                           size);

  LayoutTexture texture;
  memset(&texture, 0, sizeof(texture));
  ((void (*)(void*, const void*, u32, u32))address::kLayoutMakeImage)(
      &texture, image, size, kMemoryFromFile);
  if (!IsTypeIcon(texture)) {
    ui::Log::Print(u"type icon %u: the game refused the image (%ux%u)", type,
                   texture.width, texture.height);
    return nullptr;
  }
  textures[type] = texture;
  states[type] = State::kReady;
  return &textures[type];
}

// The game asks for an image of a resource. The numbers of the type icons are
// 24 to 41 in the resource of the common graphic font: after that, the game
// has nothing (the number of a type that is not in the game reads memory that
// is not a texture). The hook looks at the first and the last type icons: when
// they are the type icons, the resource is the one of the common graphic font,
// and the numbers after them are for the types that the plugin adds.
void GetImageHook(void* out, const void* self, u32 id) {
  const u32 type = id >= kFirstIcon ? id - kFirstIcon : kMaxTypes;
  const bool is_new = type >= kGameTypeCount && type < kMaxTypes;
  const bool wants_image = FindImageType(type) != nullptr;
  if (!is_new && !wants_image) {
    get_image_hook(out, self, id);
    return;
  }
  LayoutTexture first;
  LayoutTexture last;
  get_image_hook(&first, self, kFirstIcon);
  get_image_hook(&last, self, kFirstIcon + kGameTypeCount - 1);
  if (!IsTypeIcon(first) || !IsTypeIcon(last)) {
    // Another resource: the number is a normal image.
    get_image_hook(out, self, id);
    return;
  }
  if (wants_image) {
    const LayoutTexture* texture = LoadTypeImage(type);
    if (texture != nullptr) {
      memcpy(out, texture, sizeof(*texture));
      return;
    }
  }
  if (!is_new) {
    // A type of the game without a good image: its own icon.
    get_image_hook(out, self, id);
    return;
  }
  // The icon of the type Fairy for a new type that has no image.
  memcpy(out, &last, sizeof(last));
}

// Some layouts (the summary, the party menu) show the icon of a type with an
// animation that swaps the texture: the frame of the animation is the number of
// the type, and the animation has 18 textures. For a new type (the frame 19 and
// more), the animation keeps its last texture (the icon of the type Fairy).
// The hook puts the texture of the type in the material after the animation,
// for each texture of the size of an icon.
void PlayMaterialAnimationHook(void* transform, u32 index, void* material) {
  play_material_animation_hook(transform, index, material);
  if (material == nullptr) return;
  const f32 frame = *reinterpret_cast<f32*>(static_cast<u8*>(transform) + 16);
  const u32 type = static_cast<u32>(frame);
  if (!(frame >= 0.0f) || static_cast<f32>(type) != frame ||
      FindImageType(type) == nullptr) {
    return;
  }

  u8* const base = static_cast<u8*>(material);
  u8* const maps = *reinterpret_cast<u8**>(base + 0x34);
  const u32 count = *reinterpret_cast<u32*>(base + 0x30) & 3;
  if (maps == nullptr) return;
  for (u32 unit = 0; unit < count; unit++) {
    u8* map = maps + unit * 32;
    if (*reinterpret_cast<u16*>(map + 8) != kIconWidth ||
        *reinterpret_cast<u16*>(map + 10) != kIconHeight) {
      continue;
    }
    const LayoutTexture* texture = LoadTypeImage(type);
    if (texture == nullptr) return;
    // The fields of a texture of a material, as the game writes them.
    memcpy(map, texture, 16);
    u32 bits;
    memcpy(&bits, map + 16, sizeof(bits));
    bits = (bits & ~0xF00u) |
           ((static_cast<u32>(texture->format) << 8) & 0xF00u);
    memcpy(map + 16, &bits, sizeof(bits));
    ((void (*)(void*))address::kLayoutRefreshImage)(map);
  }
}

// The game sets the frame of an animation: the plugin remembers the last one.
void SeekAnimationHook(void* layout, u32 animation, f32 frame) {
  last_frame = {layout, animation, frame};
  seek_animation_hook(layout, animation, frame);
}

// The game shows the icon of a type in a pane. The buttons of a battle pick
// their colour with the frame of an animation, which is the number of the type:
// a new type is after the last frame. When the frame of the colour was set just
// before, the frame becomes the one of the colour type of the new type.
void ShowTypeIconHook(void* self, void* layout, u32 pane, u32 type) {
  show_type_icon_hook(self, layout, pane, type);
  if (type < kGameTypeCount || type >= kMaxTypes) return;
  const battle::TypeSpec* spec = battle::GameExtension::FindType(type);
  if (spec != nullptr && last_frame.layout == layout &&
      last_frame.frame == static_cast<f32>(type)) {
    seek_animation_hook(layout, last_frame.animation,
                         static_cast<f32>(static_cast<u32>(spec->colour_type)));
    last_frame.frame = -1.0f;
  }
}
} // namespace

void TypeIcon::Initialize() {
#ifdef GAME_ORAS
  get_image_hook.Install(address::kLayoutGetImage, GetImageHook);
  seek_animation_hook.Install(address::kLayoutSeekAnimation,
                               SeekAnimationHook);
  show_type_icon_hook.Install(battle::address::kShowTypeIcon, ShowTypeIconHook,
                             battle::address::kVtable);
  play_material_animation_hook.Install(address::kLayoutPlayMaterialAnimation,
                                PlayMaterialAnimationHook);
#endif
}

u32 TypeIcon::ReadFile(const c16* path, u8* buffer, u32 capacity) {
  const u32 read = sys::File::ReadAll(path, buffer, capacity);
  if (read <= kFooterSize) return 0;
  // The footer is at the end of the file, and it tells the size of the file.
  // The read can give a size that is bigger than the file.
  for (u32 end = read; end >= kFooterSize; end--) {
    u32 signature;
    u32 file_size;
    memcpy(&signature, buffer + end - kFooterSize, sizeof(signature));
    memcpy(&file_size, buffer + end - kFooterSize + kFooterFileSizeOffset,
           sizeof(file_size));
    if (signature == kFooterSignature && file_size == end) return end;
  }
  return 0;
}

} // namespace renderer
