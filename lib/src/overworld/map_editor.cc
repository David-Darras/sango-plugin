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
 * @file map_editor.cc
 * @brief Changes the attributes of many tiles of a world.
 *
 * The declarations are in overworld/patch/map_editor.h.
 */

#include "overworld/patch/map_editor.h"

#include <cstdio>
#include <cstring>

#include "core/native/process_manager.h"
#include "core/utils.h"
#include "overworld/address.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/tile.h"
#include "overworld/native/world_layout.h"
#include "system/native/file.h"

namespace overworld {

namespace {
const c16 kFolder[] = u"sdmc:/sango";
const c16 kMapsFolder[] = u"sdmc:/sango/maps";

// Writes the path of the file of a world.
void MakePath(u16 layout, c16* out) {
  core::Utils::Format(out, u"sdmc:/sango/maps/layout_%d.tiles",
                      static_cast<int>(layout));
}
constexpr s32 kTextSide = 128;
constexpr u32 kTextLimit = 20000;
constexpr u32 kPollFrames = 45;

// The text path of a world.
void MakeTextPath(u16 layout, c16* out) {
  core::Utils::Format(out, u"sdmc:/sango/maps/layout_%d.txt",
                      static_cast<int>(layout));
}

u32 HashBytes(const u8* data, u32 size) {
  u32 hash = 2166136261u ^ size;
  for (u32 i = 0; i < size; i++) {
    hash ^= data[i];
    hash *= 16777619u;
  }
  return hash;
}

c8 ClassOf(u32 raw) {
  const u32 low = MapEditor::DecodeAttr(raw) & 7;
  if (low & 1) return '#';
  if (low & 2) return '~';
  if (low & 4) return ',';
  return '.';
}

// The attribute that gives a tile the class of a character.
u32 ApplyClass(u32 raw, c8 wanted) {
  if (ClassOf(raw) == wanted) return raw;
  switch (wanted) {
    case '#':
      return MapEditor::SetLowFlags(raw, 1);
    case '~':
      return MapEditor::SetLowFlags(raw, 2);
    case ',':
      return MapEditor::SetLowFlags(raw, 4);
    case '.':
      return MapEditor::SetLowFlags(raw, 0);
    default:
      return raw;
  }
}
} // namespace

u32 MapEditor::DecodeAttr(u32 raw) {
#ifdef GAME_ORAS
  const u16* table = reinterpret_cast<const u16*>(address::kTileFlagTable);
  return (raw & ~7u) | table[raw >> 24];
#else
  return raw;
#endif
}

u32 MapEditor::SetLowFlags(u32 raw, u32 low) {
  low &= 7;
  if ((DecodeAttr(raw) & 7) == low) return raw;
#ifdef GAME_ORAS
  const u16* table = reinterpret_cast<const u16*>(address::kTileFlagTable);
  // The ground whose flags are exactly these flags, or else a ground whose
  // low flags are these flags.
  s32 exact = -1;
  s32 partial = -1;
  for (u32 id = 0; id < 256; id++) {
    if ((table[id] & 7u) != low) continue;
    if (partial < 0) partial = static_cast<s32>(id);
    if (table[id] == low && exact < 0) exact = static_cast<s32>(id);
  }
  const s32 id = exact >= 0 ? exact : partial;
  if (id < 0) return raw;
  return (raw & 0x00FFFFFFu) | (static_cast<u32>(id) << 24);
#else
  return (raw & ~7u) | low;
#endif
}

u32 MapEditor::GetLayout() { return GetInstance().loaded_layout_; }
bool MapEditor::IsDirty() { return GetInstance().is_dirty_; }
u32 MapEditor::GetRunCount() { return GetInstance().run_count_; }
bool MapEditor::IsFull() { return GetInstance().is_full_; }
bool MapEditor::CanUndo() { return GetInstance().history_cursor_ > 0; }
bool MapEditor::CanRedo() {
  const auto& self = GetInstance();
  return self.history_cursor_ < self.history_count_;
}

u32 MapEditor::GetTileCount() {
  const auto& self = GetInstance();
  u32 count = 0;
  for (u32 i = 0; i < self.run_count_; i++) count += self.runs_[i].length;
  return count;
}

void MapEditor::RefreshBlocks() {
  if (blocks_frame_ == frame_) return;
  block_count_ = TileEditor::CollectBlocks(blocks_, TileEditor::kMaxBlocks);
  blocks_frame_ = frame_;
}

int MapEditor::FindRun(s32 x, s32 z) const {
  for (u32 i = 0; i < run_count_; i++) {
    const Run& run = runs_[i];
    if (run.z == z && x >= run.x && x < static_cast<s32>(run.x) + run.length) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void MapEditor::RemoveTile(s32 x, s32 z) {
  const int index = FindRun(x, z);
  if (index < 0) return;
  Run& run = runs_[index];
  if (run.length == 1) {
    run = runs_[--run_count_];
  } else if (x == run.x) {
    run.x++;
    run.length--;
  } else if (x == static_cast<s32>(run.x) + run.length - 1) {
    run.length--;
  } else {
    // The tile is in the middle: the run becomes two runs. SetTile() checked
    // that the list has room; an undo can ask for more, and then the end of
    // the run is lost.
    Run right = run;
    right.x = static_cast<u16>(x + 1);
    right.length = static_cast<u16>(run.x + run.length - (x + 1));
    run.length = static_cast<u16>(x - run.x);
    if (run_count_ < kMaxRuns) runs_[run_count_++] = right;
  }
}

void MapEditor::AddTile(s32 x, s32 z, u32 attr) {
  int left = -1;
  int right = -1;
  for (u32 i = 0; i < run_count_; i++) {
    const Run& run = runs_[i];
    if (run.z != z || run.attr != attr) continue;
    if (static_cast<s32>(run.x) + run.length == x) left = static_cast<int>(i);
    if (static_cast<s32>(run.x) == x + 1) right = static_cast<int>(i);
  }
  if (left >= 0 && right >= 0) {
    runs_[left].length =
        static_cast<u16>(runs_[left].length + 1 + runs_[right].length);
    runs_[right] = runs_[--run_count_];
  } else if (left >= 0) {
    runs_[left].length++;
  } else if (right >= 0) {
    runs_[right].x--;
    runs_[right].length++;
  } else if (run_count_ < kMaxRuns) {
    Run& run = runs_[run_count_++];
    run.z = static_cast<u16>(z);
    run.x = static_cast<u16>(x);
    run.length = 1;
    run._0 = 0;
    run.attr = attr;
  }
}

bool MapEditor::ReadAttr(s32 tile_x, s32 tile_z, u32& attr) {
  auto& self = GetInstance();
  self.RefreshBlocks();
  return TileEditor::ReadRaw(self.blocks_, self.block_count_, tile_x, tile_z,
                             &attr);
}

void MapEditor::BeginStroke() {
  auto& self = GetInstance();
  if (self.is_in_stroke_) return;
  self.stroke_++;
  self.is_in_stroke_ = true;
  self.is_full_ = false;
}

void MapEditor::EndStroke() { GetInstance().is_in_stroke_ = false; }

void MapEditor::Push(const Change& change) {
  if (history_cursor_ < history_count_) history_count_ = history_cursor_;
  if (history_count_ >= kMaxHistory) {
    // The oldest changes leave.
    const u32 drop = kMaxHistory / 4;
    memmove(history_, history_ + drop, (history_count_ - drop) * sizeof(Change));
    history_count_ -= drop;
  }
  history_[history_count_++] = change;
  history_cursor_ = history_count_;
}

bool MapEditor::SetTile(s32 tile_x, s32 tile_z, u32 attr) {
  auto& self = GetInstance();
  if (tile_x < 0 || tile_z < 0) return false;
  self.RefreshBlocks();
  u32* slot = TileEditor::Slot(self.blocks_, self.block_count_, tile_x, tile_z);
  if (slot == nullptr) return false;
  const int run_index = self.FindRun(tile_x, tile_z);
  const bool had_run = run_index >= 0;
  const u32 old_attr = *slot;
  if (!had_run && old_attr == attr) return true;
  if (had_run && self.runs_[run_index].attr == attr && old_attr == attr) {
    return true;
  }
  // A change can split a run and add a run: keep room for two.
  if (self.run_count_ + 2 > kMaxRuns) {
    self.is_full_ = true;
    return false;
  }
  if (!self.is_in_stroke_) BeginStroke();

  Change change;
  change.x = static_cast<u16>(tile_x);
  change.z = static_cast<u16>(tile_z);
  change.old_attr = old_attr;
  change.new_attr = attr;
  change.stroke = self.stroke_;
  change.had_run = had_run ? 1 : 0;
  change._0 = 0;
  self.Push(change);

  self.WriteTile(tile_x, tile_z, attr, true);
  return true;
}

void MapEditor::WriteTile(s32 x, s32 z, u32 attr, bool has_run) {
  RefreshBlocks();
  u32* slot = TileEditor::Slot(blocks_, block_count_, x, z);
  if (slot != nullptr) *slot = attr;
  RemoveTile(x, z);
  if (has_run) AddTile(x, z, attr);
  is_dirty_ = true;
}

bool MapEditor::Undo() {
  auto& self = GetInstance();
  if (self.history_cursor_ == 0) return false;
  const u16 stroke = self.history_[self.history_cursor_ - 1].stroke;
  while (self.history_cursor_ > 0 &&
         self.history_[self.history_cursor_ - 1].stroke == stroke) {
    const Change& change = self.history_[--self.history_cursor_];
    self.WriteTile(change.x, change.z, change.old_attr, change.had_run != 0);
  }
  return true;
}

bool MapEditor::Redo() {
  auto& self = GetInstance();
  if (self.history_cursor_ >= self.history_count_) return false;
  const u16 stroke = self.history_[self.history_cursor_].stroke;
  while (self.history_cursor_ < self.history_count_ &&
         self.history_[self.history_cursor_].stroke == stroke) {
    const Change& change = self.history_[self.history_cursor_++];
    self.WriteTile(change.x, change.z, change.new_attr, true);
  }
  return true;
}

void MapEditor::ApplyRuns() {
  RefreshBlocks();
  const s32 tiles = static_cast<s32>(WorldLayout::kTilesPerBlockSide);
  for (u32 i = 0; i < run_count_; i++) {
    const Run& run = runs_[i];
    u32 done = 0;
    while (done < run.length) {
      const s32 x = static_cast<s32>(run.x) + static_cast<s32>(done);
      const TileEditor::Block* block =
          TileEditor::Find(blocks_, block_count_, x, run.z);
      if (block == nullptr) {
        done++;
        continue;
      }
      const u32 local_x = static_cast<u32>(x - block->x * tiles);
      const u32 local_z = static_cast<u32>(run.z - block->z * tiles);
      u32 count = block->width - local_x;
      if (count > run.length - done) count = run.length - done;
      u32* destination = &block->attrs[local_x + local_z * block->width];
      for (u32 k = 0; k < count; k++) destination[k] = run.attr;
      done += count;
    }
  }
}

void MapEditor::Update() {
  auto& self = GetInstance();
  self.frame_++;
  if (!core::ProcessManager::IsOverworldActive()) return;
  const u16 layout = WorldLayout::GetInstance().id;
  if (layout != self.loaded_layout_) self.SwitchLayout(layout);
  if (self.is_live && self.frame_ % kPollFrames == 0) self.PollText();
  if (!self.is_enabled || self.run_count_ == 0) return;
  if (self.frame_ % 3 != 0) return;
  self.ApplyRuns();
}

void MapEditor::SwitchLayout(u16 layout) {
  if (loaded_layout_ != 0xFFFF && is_dirty_ && auto_save) Save();
  run_count_ = 0;
  history_count_ = 0;
  history_cursor_ = 0;
  is_dirty_ = false;
  is_full_ = false;
  loaded_layout_ = layout;
  Load();
}

bool MapEditor::Load() {
  c16 path[64];
  MakePath(loaded_layout_, path);
  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) return false;
  u32 header[3] = {};
  if (file.Read(header, sizeof(header)) != static_cast<s32>(sizeof(header))) {
    return false;
  }
  if (header[0] != kMagic || header[1] != loaded_layout_ ||
      header[2] > kMaxRuns) {
    return false;
  }
  const u32 size = header[2] * sizeof(Run);
  if (size != 0 &&
      file.Read(runs_, size) != static_cast<s32>(size)) {
    return false;
  }
  run_count_ = header[2];
  return true;
}

bool MapEditor::Save() {
  auto& self = GetInstance();
  if (self.loaded_layout_ == 0xFFFF) return false;
  c16 path[64];
  MakePath(self.loaded_layout_, path);
  if (self.run_count_ == 0) {
    sys::File::Delete(path);
    self.is_dirty_ = false;
    return true;
  }
  sys::File::CreateDirectory(kFolder);
  sys::File::CreateDirectory(kMapsFolder);
  sys::File file(path, true);
  if (!file.IsOpen()) return false;
  const u32 header[3] = {kMagic, self.loaded_layout_, self.run_count_};
  file.Write(header, sizeof(header));
  file.Write(self.runs_, self.run_count_ * sizeof(Run));
  self.is_dirty_ = false;
  return true;
}

bool MapEditor::ExportText() {
  auto& self = GetInstance();
  if (self.loaded_layout_ == 0xFFFF) return false;
  auto& player = ModelManager::GetInstance().GetPlayer();
  s32 x0 = static_cast<s32>(player.map_pos.coords.x) - kTextSide / 2;
  s32 z0 = static_cast<s32>(player.map_pos.coords.z) - kTextSide / 2;
  if (x0 < 0) x0 = 0;
  if (z0 < 0) z0 = 0;

  static c8 text[kTextLimit];
  int used = snprintf(
      text, 400,
      "# Sango map, world %u. One character for each tile.\n"
      "# # wall   ~ water   , wild Pokemon   . plain   ? not loaded\n"
      "# Change the characters and save the file: the game applies them.\n"
      "origin %d %d\n",
      (unsigned)self.loaded_layout_, (int)x0, (int)z0);
  if (used < 0) return false;
  u32 length = static_cast<u32>(used);
  for (s32 row = 0; row < kTextSide; row++) {
    for (s32 column = 0; column < kTextSide; column++) {
      u32 attr;
      text[length++] = ReadAttr(x0 + column, z0 + row, attr) ? ClassOf(attr) : '?';
    }
    text[length++] = '\n';
  }

  c16 path[64];
  MakeTextPath(self.loaded_layout_, path);
  sys::File::CreateDirectory(kFolder);
  sys::File::CreateDirectory(kMapsFolder);
  sys::File file(path, true);
  if (!file.IsOpen()) return false;
  file.Write(text, length);
  self.text_hash_ = HashBytes(reinterpret_cast<const u8*>(text), length);
  self.has_text_baseline_ = true;
  return true;
}

bool MapEditor::ImportText(u32* changed) {
  auto& self = GetInstance();
  if (changed != nullptr) *changed = 0;
  if (self.loaded_layout_ == 0xFFFF) return false;
  c16 path[64];
  MakeTextPath(self.loaded_layout_, path);
  static c8 text[kTextLimit];
  const u32 size = sys::File::ReadAll(path, text, kTextLimit - 1);
  if (size == 0) return false;
  text[size] = '\0';
  self.text_hash_ = HashBytes(reinterpret_cast<const u8*>(text), size);
  self.has_text_baseline_ = true;

  s32 x0 = 0;
  s32 z0 = 0;
  s32 row = 0;
  u32 count = 0;
  bool has_origin = false;
  BeginStroke();
  const c8* p = text;
  while (*p != '\0') {
    const c8* end = p;
    while (*end != '\0' && *end != '\n') end++;
    if (*p == '#' && p[1] == ' ') {
      // A comment line.
    } else if (strncmp(p, "origin", 6) == 0) {
      int a = 0;
      int b = 0;
      if (sscanf(p + 6, "%d %d", &a, &b) == 2) {
        x0 = a;
        z0 = b;
        has_origin = true;
      }
    } else if (has_origin && end > p) {
      for (const c8* c = p; c < end && c - p < kTextSide; c++) {
        if (*c == '?' || *c == '\r') continue;
        u32 attr;
        const s32 x = x0 + static_cast<s32>(c - p);
        const s32 z = z0 + row;
        if (!ReadAttr(x, z, attr)) continue;
        const u32 wanted = ApplyClass(attr, *c);
        if (wanted == attr) continue;
        if (!SetTile(x, z, wanted)) break;
        count++;
      }
      row++;
    }
    p = *end == '\n' ? end + 1 : end;
  }
  EndStroke();
  if (changed != nullptr) *changed = count;
  return true;
}

void MapEditor::PollText() {
  c16 path[64];
  MakeTextPath(loaded_layout_, path);
  static u8 buffer[kTextLimit];
  const u32 size = sys::File::ReadAll(path, buffer, kTextLimit);
  if (size == 0) return;
  const u32 hash = HashBytes(buffer, size);
  if (!has_text_baseline_) {
    text_hash_ = hash;
    has_text_baseline_ = true;
    return;
  }
  if (hash != text_hash_) ImportText(nullptr);
}

void MapEditor::ClearAll() {
  auto& self = GetInstance();
  self.run_count_ = 0;
  self.history_count_ = 0;
  self.history_cursor_ = 0;
  self.is_dirty_ = false;
  self.is_full_ = false;
  if (self.loaded_layout_ != 0xFFFF) {
    c16 path[64];
    MakePath(self.loaded_layout_, path);
    sys::File::Delete(path);
  }
}

} // namespace overworld
