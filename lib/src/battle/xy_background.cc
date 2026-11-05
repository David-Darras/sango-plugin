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
 * @file xy_background.cc
 * @brief Chooses the pieces of the background of a battle: pieces of ORAS or
 *        of Pokémon X and Y.
 *
 * The declarations are in battle/patch/xy_background.h.
 */

#include "battle/patch/xy_background.h"

#include "core/constant/archive_id.h"
#include "core/native/process_manager.h"
#include "core/patch/archive.h"
#include "system/native/file.h"
#include "ui/log.h"

namespace battle {

namespace {
constexpr u32 kMaxValues = 512;
const c16* const kChoicePaths[XyBackground::kKindCount] = {
    u"sdmc:/sango/battle_background/choices_G.bin",
    u"sdmc:/sango/battle_background/choices_F.bin",
    u"sdmc:/sango/battle_background/choices_B.bin",
    u"sdmc:/sango/battle_background/choices_O.bin",
};
const c8* const kKindNames[XyBackground::kKindCount] = {
    "bgground", "bgfar", "bgback", "bgplate"};
}  // namespace

const c8* XyBackground::GetKindName(u32 kind) {
  return kind < kKindCount ? kKindNames[kind] : "";
}

void XyBackground::Apply() {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  // The change of the last battle, if it is still there, goes first.
  RemoveApplied();
  for (u32 kind = 0; kind < kKindCount; kind++) {
    if (self.choices[kind] == 0) continue;

    // The file of the choices: the files of the entries, then the files of
    // ORAS that the game can ask for.
    static u16 values[kMaxValues];
    sys::File file;
    file.Open(kChoicePaths[kind], sys::File::kRead);
    if (!file.IsOpen()) {
      ui::Log::Print(u"background: no choices file for the kind %u", kind);
      continue;
    }
    const s32 size = file.Read(values, sizeof(values));
    file.Close();
    if (size < 4) continue;
    const u32 count = static_cast<u32>(size) / sizeof(u16);
    const u32 entries = values[0];
    if (1 + entries >= count || self.choices[kind] >= entries) continue;
    const u32 target = values[1 + self.choices[kind]];
    const u32 sources = values[1 + entries];
    if (1 + entries + 1 + sources > count) continue;

    // A new file that is not in the archive and not on the SD card makes the
    // game wait for ever.
    if (target >= kFirstNewFile &&
        !core::Archive::HasSdFile(core::ArchiveId::kBattleBackground,
                                  target)) {
      ui::Log::Print(u"background: no file %u in sango/battle_background",
                     target);
      continue;
    }
    for (u32 i = 0; i < sources; i++) {
      const u32 from = values[2 + entries + i];
      if (from == target || self.applied_count_ >= kMaxApplied) continue;
      if (!core::Archive::ReplaceFile(core::ArchiveId::kBattleBackground, from,
                                      target)) {
        continue;
      }
      self.applied_files_[self.applied_count_++] = from;
    }
  }
  self.is_applied_ = self.applied_count_ > 0;
  self.is_battle_seen_ = false;
#endif
}

void XyBackground::RemoveApplied() {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  for (u32 i = 0; i < self.applied_count_; i++) {
    core::Archive::RemoveReplacement(core::ArchiveId::kBattleBackground,
                                     self.applied_files_[i]);
  }
  self.applied_count_ = 0;
  self.is_applied_ = false;
#endif
}

void XyBackground::Update() {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  if (!self.is_applied_) return;
  if (core::ProcessManager::IsBattleActive()) {
    self.is_battle_seen_ = true;
    return;
  }
  if (self.is_battle_seen_ && core::ProcessManager::IsOverworldActive()) {
    RemoveApplied();
    self.is_battle_seen_ = false;
  }
#endif
}
} // namespace battle
