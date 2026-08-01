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
#include "common.h"
#include "pokemon_utils.h"

struct PokemonCoreData {
  // HEADER [0x08 bytes]
  u32 encryption_key; // 04
  u16 _0; // 06
  u16 checksum; // 08

  // BLOCK 1 [0x38 bytes]
  u16 species; // 02
  u16 item; // 04

  u32 id; // 08

  u32 experience; // 0C

  u8 ability; // 0D
  u8 ability_flags; // 0E
  u8 super_training_bag_hit_count; // 0F
  u8 super_training_bag_type; // 10

  u32 shiny_id; // 14

  u8 nature; // 15
  union {
    u8 event_gender_form_flags; // 16
    struct {
      u8 event : 1;
      u8 gender : 2;
      u8 form : 5;
    };
  };

  u8 ev_hp; // 17
  u8 ev_attack; // 18

  u8 ev_defense; // 19
  u8 ev_speed; // 1A
  u8 ev_special_attack; // 1B
  u8 ev_special_defense; // 1C

  struct {
    u8 cool; // 1D
    u8 beautiful; // 1E
    u8 cute; // 1F
    u8 smart; // 20

    u8 tough; // 21
    u8 sheen; // 22
  } contest;

  u8 box_mark_flags; // 23
  u8 pokerus_flags; // 24

  u32 super_training_flags; // 28

  u32 ribbon_flags[2]; // 2C, 30

  u16 other_ribbon_flags; // 32
  u16 other_super_ribbon_flags; // 34

  u32 furfrou_remaining_trim_days; // 38

  // BLOCK 2  [0x38 bytes]
  c16 nickname[13]; // 1A
  u16 moves[4]; // 22
  u8 pp[4]; // 26
  u8 pp_up_count[4]; // 2A
  u16 egg_moves[4]; // 32
  u8 secret_super_training_flags; // 33
  u8 _1; // 34

  union {
    u32 iv_flags; // 38
    struct {
      u32 iv_hp : 5;
      u32 iv_attack : 5;
      u32 iv_defense : 5;
      u32 iv_speed : 5;
      u32 iv_special_attack : 5;
      u32 iv_special_defense : 5;
      u32 is_egg : 1;
      u32 use_nickname : 1;
    };
  };

  // BLOCK 3 [0x38 bytes]
  c16 first_trainer_name[13]; // 1A
  u8 first_trainer_gender; // 1B
  u8 __0;
  u16 __1[7];
  u8 __2;
  u8 happiness;
  u16 __4[6];

  // BLOCK 4 [0x38 bytes]
  c16 original_trainer_name[13]; // 1A
  union {
    u8 training_friendship; // 1B
    u8 remaining_steps_before_hatch;
  };

  u8 contest_friendship; // 1C
  u8 _3[5]; // 21

  u8 egg_year; // 22
  u8 egg_month; // 23
  u8 egg_day; // 24

  u8 met_year; // 25
  u8 met_month; // 26
  u8 met_day; // 27

  u8 _4; // 28

  u16 egg_location; // 2A
  u16 met_location; // 2C
  u8 ball; // 2D
  u8 captured_level_owner_gender_flags; // 2E

  u8 _5; // 2F
  u8 game_version; // 30
  u8 country; // 31
  u8 region; // 32
  u8 console_region; // 33
  u8 language; // 34
  u32 _6; // 38

  static constexpr u32 kBlockSize = 0x38;
  static constexpr u32 kBlockCount = 4;

  INLINE void* GetBlock(u32 index) {
    uptr addr = (uptr)this;
    return (void*)(addr + 8 + index * kBlockSize);
  }

  void SetNickname(const c16* nickname) {
    u32 src_len = 0;
    while (src_len < 12 && nickname[src_len] != u'\0') {
      this->nickname[src_len] = nickname[src_len];
      src_len++;
    }
    this->nickname[src_len] = u'\0';
    this->use_nickname = true;
  }

  void ResetNickname() {
    c16 buffer[13];
    ((void(*)(c16*, u16))0x00139E40)(buffer, species);
    SetNickname(buffer);
    use_nickname = false;
  }

  void SetShiny(bool is_shiny) {
    if (is_shiny) {
      PokemonUtils::ConvertToShiny(id, &shiny_id);
    } else {
      PokemonUtils::ConvertToNormal(id, &shiny_id);
    }
  }

  void SetLevel(u8 level) {
    experience = PokemonUtils::GetExperienceFromLevel(species, form, level);
  }

  void SetMaxIVs() {
    iv_hp = 31;
    iv_attack = 31;
    iv_defense = 31;
    iv_speed = 31;
    iv_special_attack = 31;
    iv_special_defense = 31;
  }

  void SetStats(u8 hp, u8 atk, u8 def, u8 sp_atk,
                u8 sp_def, u8 spd) {
    SetMaxIVs();
    ev_hp = hp;
    ev_attack = atk;
    ev_defense = def;
    ev_special_attack = sp_atk;
    ev_special_defense = sp_def;
    ev_speed = spd;
  }

  void SetMoves(u16 move1, u16 move2, u16 move3, u16 move4) {
    moves[0] = move1;
    moves[1] = move2;
    moves[2] = move3;
    moves[3] = move4;
  }
};