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
 * @file type_chart.cc
 * @brief Reads and changes the type chart.
 *
 * The declarations are in battle/patch/type_chart.h.
 */

#include "battle/patch/type_chart.h"
#include "battle/address.h"
#include "battle/patch/game_extension.h"
#include "core/hook.h"
#include "core/native/process_manager.h"

namespace battle {

namespace {
// The results of the game for the effect of a type.
constexpr u32 kResultNone = 0;
constexpr u32 kResultHalf = 6;
constexpr u32 kResultNormal = 7;
constexpr u32 kResultDouble = 8;

constexpr u32 kNoType = static_cast<u32>(TypeId::kCount);
constexpr u32 kMaxTypes = GameExtension::kMaxTypes;
constexpr u32 kFirstCustom = static_cast<u32>(TypeId::kFirstCustom);

// The cells that have a new type: [attacking][defending], with the number of
// the type. The cells of the types of the game are in the memory of the game.
TypeMultiplier custom_cells[kMaxTypes][kMaxTypes];
bool is_chart_ready = false;

constexpr TypeMultiplier k0 = TypeMultiplier::k0;
constexpr TypeMultiplier k05 = TypeMultiplier::k05;
constexpr TypeMultiplier k1 = TypeMultiplier::k1;
constexpr TypeMultiplier k2 = TypeMultiplier::k2;
#include "battle/data/type_chart.inc"

// The cells of the types of the game: a copy of the chart of the game that the
// plugin changes. The memory of the game has the real type chart only when the
// battle code is loaded: Set() writes it then, and PatchLoad() writes the copy
// each time that the battle code loads.
TypeMultiplier game_cells[TypeChart::kTypeCount][TypeChart::kTypeCount];
bool has_game_cells = false;
bool is_game_copy_ready = false;

void MakeGameCopy() {
  if (is_game_copy_ready) return;
  is_game_copy_ready = true;
  for (u32 row = 0; row < TypeChart::kTypeCount; row++) {
    for (u32 col = 0; col < TypeChart::kTypeCount; col++) {
      game_cells[row][col] = kOriginalChart[row][col];
    }
  }
}

bool IsBattleLoaded() {
  return core::ProcessManager::GetInstance().IsCurrentProcess(
      address::kVtable);
}

core::Hook<u32(u32, u32)> type_matchup_hook;

void MakeChart() {
  if (is_chart_ready) return;
  is_chart_ready = true;
  for (auto& row : custom_cells) {
    for (auto& cell : row) cell = TypeMultiplier::k1;
  }
}

// The cell of the plugin for a pair that has a new type, or null.
TypeMultiplier* FindCustomCell(u32 attacking, u32 defending) {
  if (attacking < kFirstCustom && defending < kFirstCustom) return nullptr;
  if (attacking >= kMaxTypes || defending >= kMaxTypes ||
      attacking == kNoType || defending == kNoType) {
    return nullptr;
  }
  return &custom_cells[attacking][defending];
}

// The game asks for the effect of a type against a type. The types of the game
// go to the game. A pair with a new type gets its result here, and the
// inverse battle changes it as the game does.
u32 TypeMatchupHook(u32 attacking, u32 defending) {
  if (attacking < kFirstCustom && defending < kFirstCustom) {
    return type_matchup_hook(attacking, defending);
  }
  MakeChart();
  const TypeMultiplier* cell = FindCustomCell(attacking, defending);
  if (cell == nullptr) return kResultNormal;
  const bool is_inverse =
      *reinterpret_cast<volatile s8*>(address::kInverseBattleFlag) != 0;
  switch (*cell) {
    case TypeMultiplier::k0:
      return is_inverse ? kResultDouble : kResultNone;
    case TypeMultiplier::k05:
      return is_inverse ? kResultDouble : kResultHalf;
    case TypeMultiplier::k2:
      return is_inverse ? kResultHalf : kResultDouble;
    default:
      return kResultNormal;
  }
}
} // namespace

void TypeChart::Initialize() {
#ifdef GAME_ORAS
  MakeChart();
  type_matchup_hook.Install(address::kTypeMatchup, TypeMatchupHook,
                             address::kVtable);
#endif
}

void TypeChart::PatchLoad() {
  if (!has_game_cells) return;
  MakeGameCopy();
  for (u32 row = 0; row < kTypeCount; row++) {
    for (u32 col = 0; col < kTypeCount; col++) {
      GetTable()[row][col] = game_cells[row][col];
    }
  }
}

TypeChart::Row* TypeChart::GetTable() {
  return reinterpret_cast<Row*>(address::kTypeChart);
}

void TypeChart::Set(TypeId attacking_type, TypeId defending_type,
                    TypeMultiplier value) {
  const u32 row = static_cast<u32>(attacking_type);
  const u32 col = static_cast<u32>(defending_type);
  MakeChart();
  if (TypeMultiplier* cell = FindCustomCell(row, col)) {
    *cell = value;
    return;
  }
  if (row >= kTypeCount || col >= kTypeCount) return;
  MakeGameCopy();
  game_cells[row][col] = value;
  has_game_cells = true;
  if (IsBattleLoaded()) GetTable()[row][col] = value;
}

TypeMultiplier TypeChart::Get(TypeId attacking_type,
                                     TypeId defending_type) {
  const u32 row = static_cast<u32>(attacking_type);
  const u32 col = static_cast<u32>(defending_type);
  MakeChart();
  if (const TypeMultiplier* cell = FindCustomCell(row, col)) return *cell;
  if (row >= kTypeCount || col >= kTypeCount) return TypeMultiplier::k1;
  MakeGameCopy();
  return game_cells[row][col];
}

void TypeChart::SetTable(const TypeMultiplier* table, u32 type_count) {
  if (table == nullptr || type_count < kTypeCount ||
      type_count > kMaxTypes - 1) {
    return;
  }
  // The numbers of the table: the types of the game, then the new types (the
  // number 18 of the table is the type 19, and so on).
  auto to_type = [](u32 slot) {
    return static_cast<TypeId>(slot < kTypeCount ? slot : slot + 1);
  };
  for (u32 attacking = 0; attacking < type_count; attacking++) {
    for (u32 defending = 0; defending < type_count; defending++) {
      const TypeMultiplier value = table[attacking * type_count + defending];
      Set(to_type(attacking), to_type(defending), value);
    }
  }
}

} // namespace battle
