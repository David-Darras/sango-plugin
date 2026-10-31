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
 * @file hack_save.cc
 * @brief The flags and the variables of the ROM hack in the save data.
 *
 * The declarations are in savedata/patch/hack_save.h.
 */

#include "savedata/patch/hack_save.h"

#include "core/data_folder.h"
#include "savedata/native/savedata.h"
#include "ui/log.h"

namespace savedata {

/// The reserved segment: a header, the flags, then the variables.
struct HackSave::Layout {
  u32 magic; ///< kMagic: the library wrote the segment.
  u32 product; ///< The hash of the data folder of the product.
  u16 version; ///< The version of this layout.
  u16 _0;
  u32 _1;
  u8 flags[kFlagCount / 8];
  u16 variables[kVariableCount];
};

namespace {
// "SNGO" in the save data.
constexpr u32 kMagic = 0x4F474E53;
constexpr u16 kLayoutVersion = 1;

// The product: the hash of its data folder (sdmc:/sango/<product>/).
u32 GetProductHash() {
  static const c8 kFolder[] = SANGO_DATA_TEXT("");
  u32 hash = 2166136261u;
  for (u32 i = 0; kFolder[i] != '\0'; i++) {
    hash ^= static_cast<u8>(kFolder[i]);
    hash *= 16777619u;
  }
  return hash;
}

bool IsEmpty(const u8* data, u32 size) {
  for (u32 i = 0; i < size; i++) {
    if (data[i] != 0) return false;
  }
  return true;
}

bool has_logged_other_hack = false;
bool has_logged_unknown_data = false;
} // namespace

HackSave::Layout* HackSave::GetLayout() {
  static_assert(sizeof(Layout) <= SaveData::kReservedSize,
                "The flags and the variables must fit in the reserved segment");
  u8* segment = SaveData::GetInstance().GetReservedSegment();
  if (segment == nullptr) return nullptr;
  auto* layout = reinterpret_cast<Layout*>(segment);

  if (layout->magic != kMagic) {
    // A new game, or a save data without the library: the game fills the
    // segment with zeros. Other data: do not change it.
    if (!IsEmpty(segment, SaveData::kReservedSize)) {
      if (!has_logged_unknown_data) {
        ui::Log::Print(u"save: the reserved data is not empty");
        has_logged_unknown_data = true;
      }
      return nullptr;
    }
    layout->magic = kMagic;
    layout->product = GetProductHash();
    layout->version = kLayoutVersion;
  }

  if (layout->product != GetProductHash() && !has_logged_other_hack) {
    ui::Log::Print(u"save: an other ROM hack wrote this save data");
    has_logged_other_hack = true;
  }
  return layout;
}

bool HackSave::IsAvailable() { return GetLayout() != nullptr; }

bool HackSave::IsFromOtherHack() {
  const Layout* layout = GetLayout();
  return layout != nullptr && layout->product != GetProductHash();
}

bool HackSave::GetFlag(u16 flag) {
  const Layout* layout = GetLayout();
  if (layout == nullptr || flag >= kFlagCount) return false;
  return (layout->flags[flag / 8] >> (flag % 8)) & 1;
}

void HackSave::SetFlag(u16 flag, bool value) {
  Layout* layout = GetLayout();
  if (layout == nullptr || flag >= kFlagCount) return;
  const u8 mask = static_cast<u8>(1 << (flag % 8));
  if (value) {
    layout->flags[flag / 8] |= mask;
  } else {
    layout->flags[flag / 8] &= static_cast<u8>(~mask);
  }
}

u16 HackSave::GetVariable(u16 variable) {
  const Layout* layout = GetLayout();
  if (layout == nullptr || variable >= kVariableCount) return 0;
  return layout->variables[variable];
}

void HackSave::SetVariable(u16 variable, u16 value) {
  Layout* layout = GetLayout();
  if (layout == nullptr || variable >= kVariableCount) return;
  layout->variables[variable] = value;
}

u16 HackSave::AddToVariable(u16 variable, u16 amount) {
  const u32 sum = static_cast<u32>(GetVariable(variable)) + amount;
  const u16 value = sum > 0xFFFF ? 0xFFFF : static_cast<u16>(sum);
  SetVariable(variable, value);
  return value;
}

} // namespace savedata
