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

#pragma once
#include <types.h>

#include "pokemon/constant/ability.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/type.h"
#include "pokemon/constant/species.h"

namespace pokemon {
struct CoreData;
}

namespace battle {
struct Pokemon {
  pokemon::CoreData* core_data;
  u32 _0;
  u32 experience;
  SpeciesId species;
  u16 max_hp;
  u16 hp;
  ItemId item;
  ItemId used_item;
  // NOTE: left raw on purpose. It is 16 bits wide while `Ability` is a
  // u8 enum, and retyping it would mean changing the field width.
  u16 base_ability;
  u8 level;
  u8 uid; // UID
  u8 _1[2];
  u32 status_condition[38];
  u8 status_condition_count[38];
  u8 _2[4 * 6 + 1];

  SpeciesId species_2;
  u16 attack;
  u16 defense;
  u16 special_attack;
  u16 special_defense;
  u16 speed;

  TypeId types[3];
  Gender gender;

  u8 stat_attack;
  u8 stat_defense;
  u8 stat_special_attack;
  u8 stat_special_defense;
  u8 stat_speed;
  u8 stat_accuracy;
  u8 stat_evasion;

  u16 ev_sum;
  u8 ev_hp;
  u8 ev_attack;
  u8 ev_defense;
  u8 ev_speed;
  u8 ev_special_attack;
  u8 ev_special_defense;
  bool has_pokerus;
  bool _3;

  struct {
    struct {
      MoveId id;
      u8 pp;
      u8 max_pp;
      u8 _0[2];
    } core, view;

    u8 _0;
  } moves[4];

  AbilityId ability;
  u8 padding;
  u16 weight;
  u8 move_count;
  Form form;
  u8 friendship;
  u8 _4[3];
  u16 turn_count;
  u8 _7[0xB6];

  void InverseStats() {
    u16 tmp = attack;
    attack = special_attack;
    special_attack = tmp;

    tmp = defense;
    defense = special_defense;
    special_defense = tmp;
  }

  void SetMetronome() {
    for (u32 j = 0; j < move_count; j++) {
      moves[j].core.id = MoveId::kMetronome;
      moves[j].core.pp = 99;
      moves[j].core.max_pp = 99;
      moves[j].view.id = MoveId::kMetronome;
      moves[j].view.pp = 99;
      moves[j].view.max_pp = 99;
    }
  }
};
}