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
 * @file species_editor.cc
 * @brief Changes the data of a species.
 *
 * The declarations are in pokemon/patch/species_editor.h.
 */

#include "pokemon/patch/species_editor.h"

#include "core/problems.h"
#include "pokemon/native/evolution_data.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/species_data.h"
#include "pokemon/patch/species_table.h"

namespace pokemon {
namespace {
// The table of the TMs and HMs of the game has 107 moves: TM01 to TM92, HM01
// to HM06, TM93 to TM100, then HM07. The bits of
// SpeciesData::technical_moves follow this order.
constexpr u32 kNoMachine = 0xFF;

u32 GetTmIndex(u8 number) {
  if (number >= 1 && number <= 92) return number - 1u;
  if (number >= 93 && number <= 100) return number + 5u;
  return kNoMachine;
}

u32 GetHmIndex(u8 number) {
  if (number >= 1 && number <= 6) return number + 91u;
  if (number == 7) return 106;
  return kNoMachine;
}

SpeciesChanges::Change MakeChange(SpeciesId species, FormId form,
                                  SpeciesChanges::Kind kind) {
  SpeciesChanges::Change change = {};
  change.species = species;
  change.form = form;
  change.kind = kind;
  return change;
}
} // namespace

// Species.

Species& Species::WithStats(u8 hp, u8 attack, u8 defense, u8 sp_attack,
                            u8 sp_defense, u8 speed) {
  auto change = MakeChange(species_, form_, SpeciesChanges::Kind::kStats);
  const u8 values[6] = {hp, attack, defense, sp_attack, sp_defense, speed};
  for (u32 i = 0; i < 6; i++) change.values[i] = values[i];
  SpeciesChanges::Add(change);
  return *this;
}

Species& Species::WithTypes(TypeId first, TypeId second) {
  auto change = MakeChange(species_, form_, SpeciesChanges::Kind::kTypes);
  change.values[0] = static_cast<u8>(first);
  change.values[1] = static_cast<u8>(second);
  SpeciesChanges::Add(change);
  return *this;
}

Species& Species::WithAbilities(AbilityId first, AbilityId second,
                                AbilityId hidden) {
  auto change = MakeChange(species_, form_, SpeciesChanges::Kind::kAbilities);
  change.values[0] = static_cast<u8>(first);
  change.values[1] = static_cast<u8>(second);
  change.values[2] = static_cast<u8>(hidden);
  SpeciesChanges::Add(change);
  return *this;
}

Species& Species::WithCatchRate(u8 rate) {
  auto change = MakeChange(species_, form_, SpeciesChanges::Kind::kCatchRate);
  change.values[0] = rate;
  SpeciesChanges::Add(change);
  return *this;
}

Species& Species::WithTm(u8 number, bool can_learn) {
  const u32 index = GetTmIndex(number);
  if (index == kNoMachine) {
    core::Problems::Add(u"species %u: TM%u does not exist",
                        static_cast<u32>(species_), number);
    return *this;
  }
  auto change = MakeChange(species_, form_, SpeciesChanges::Kind::kTm);
  change.values[0] = static_cast<u8>(index);
  change.values[1] = can_learn ? 1 : 0;
  SpeciesChanges::Add(change);
  return *this;
}

Species& Species::WithHm(u8 number, bool can_learn) {
  const u32 index = GetHmIndex(number);
  if (index == kNoMachine) {
    core::Problems::Add(u"species %u: HM%u does not exist",
                        static_cast<u32>(species_), number);
    return *this;
  }
  auto change = MakeChange(species_, form_, SpeciesChanges::Kind::kTm);
  change.values[0] = static_cast<u8>(index);
  change.values[1] = can_learn ? 1 : 0;
  SpeciesChanges::Add(change);
  return *this;
}

Species& Species::WithLearnset(const LevelMove* moves, u32 count) {
  SpeciesChanges::AddLearnset(species_, form_, moves, count);
  return *this;
}

Species& Species::WithEvolutions(const EvolutionStep* steps, u32 count) {
  SpeciesChanges::AddEvolutions(species_, steps, count);
  return *this;
}

// SpeciesChanges.

bool SpeciesChanges::IsReady() {
#ifdef GAME_ORAS
  // The new species and the new forms exist after the start, in the bigger
  // table of SpeciesTable.
  return SpeciesTable::IsBuilt();
#else
  return true;
#endif
}

void SpeciesChanges::Add(const Change& change) {
  if (IsReady()) {
    Apply(change);
    return;
  }
  auto& self = GetInstance();
  if (self.pending_count_ >= kMaxChanges) {
    core::Problems::Add(u"species %u: more than %u changes",
                        static_cast<u32>(change.species), kMaxChanges);
    return;
  }
  self.pending_[self.pending_count_++] = change;
}

void SpeciesChanges::ApplyPending() {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.pending_count_; i++) Apply(self.pending_[i]);
  self.pending_count_ = 0;
}

void SpeciesChanges::Apply(const Change& change) {
  SpeciesData& data = SpeciesData::GetInstance(change.species, change.form);
  const u8* v = change.values;
  switch (change.kind) {
    case Kind::kStats:
      data.base_hp = v[0];
      data.base_attack = v[1];
      data.base_defense = v[2];
      data.base_special_attack = v[3];
      data.base_special_defense = v[4];
      data.base_speed = v[5];
      break;
    case Kind::kTypes:
      data.type[0] = static_cast<TypeId>(v[0]);
      data.type[1] = static_cast<TypeId>(v[1]);
      break;
    case Kind::kAbilities:
      for (u32 i = 0; i < 3; i++) data.ability[i] = static_cast<AbilityId>(v[i]);
      break;
    case Kind::kCatchRate:
      data.capture_rate = v[0];
      break;
    case Kind::kTm: {
      u32& bits = data.technical_moves[v[0] / 32];
      const u32 mask = 1u << (v[0] % 32);
      bits = v[1] != 0 ? (bits | mask) : (bits & ~mask);
      break;
    }
  }
}

bool SpeciesChanges::AddLearnset(SpeciesId species, FormId form,
                                 const LevelMove* moves, u32 count) {
  auto& self = GetInstance();
  if (moves == nullptr) return false;
  if (count > Species::kMaxLearnsetMoves) {
    core::Problems::Add(u"species %u: more than %u learnset moves",
                        static_cast<u32>(species), Species::kMaxLearnsetMoves);
    count = Species::kMaxLearnsetMoves;
  }
  for (u32 i = 0; i < self.learnset_count_; i++) {
    Learnset& learnset = self.learnsets_[i];
    if (learnset.species == species && learnset.form == form) {
      learnset.moves = moves;
      learnset.count = count;
      return true;
    }
  }
  if (self.learnset_count_ >= kMaxLearnsets) {
    core::Problems::Add(u"species %u: more than %u learnsets",
                        static_cast<u32>(species), kMaxLearnsets);
    return false;
  }
  self.learnsets_[self.learnset_count_++] = {species, form, moves, count};
  return true;
}

bool SpeciesChanges::AddEvolutions(SpeciesId species,
                                   const EvolutionStep* steps, u32 count) {
  auto& self = GetInstance();
  if (steps == nullptr) return false;
  if (count > Species::kMaxEvolutions) {
    core::Problems::Add(u"species %u: more than %u evolutions",
                        static_cast<u32>(species), Species::kMaxEvolutions);
    count = Species::kMaxEvolutions;
  }
  for (u32 i = 0; i < self.evolution_count_; i++) {
    EvolutionList& list = self.evolutions_[i];
    if (list.species == species) {
      list.steps = steps;
      list.count = count;
      return true;
    }
  }
  if (self.evolution_count_ >= kMaxEvolutionLists) {
    core::Problems::Add(u"species %u: more than %u evolution lists",
                        static_cast<u32>(species), kMaxEvolutionLists);
    return false;
  }
  self.evolutions_[self.evolution_count_++] = {species, steps, count};
  return true;
}

void SpeciesChanges::PatchLearnset(SpeciesId species, FormId form) {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.learnset_count_; i++) {
    const Learnset& learnset = self.learnsets_[i];
    if (learnset.species != species || learnset.form != form) continue;
    Movepool& pool = Movepool::Object();
    if (pool.entry == nullptr) return;
    for (u32 k = 0; k < learnset.count; k++) {
      pool.entry[k].move = learnset.moves[k].move;
      pool.entry[k].level = learnset.moves[k].level;
      pool.entry[k].padding = 0;
    }
    pool.count = static_cast<u8>(learnset.count);
    return;
  }
}

void SpeciesChanges::PatchEvolutions(SpeciesId species, EvolutionData& data) {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.evolution_count_; i++) {
    const EvolutionList& list = self.evolutions_[i];
    if (list.species != species) continue;
    for (u32 k = 0; k < SIZE(data.data); k++) {
      const bool is_used = k < list.count;
      data.data[k].method =
          is_used ? list.steps[k].method : EvolutionMethod::kNone;
      data.data[k]._0 = 0;
      data.data[k].arg = is_used ? list.steps[k].value : 0;
      data.data[k].species = is_used ? list.steps[k].species : SpeciesId::kNone;
    }
    return;
  }
}

} // namespace pokemon
