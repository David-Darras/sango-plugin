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

#include "battle/constant/ai.h"
#include "core/types.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/item.h"

namespace sys {
struct String;
}

namespace battle {
struct TrainerAppearance;

struct TrainerData {
  u64 local_friend_code;
  u32 id;
  AiFlags ai_flags;
  u8 battle_effect_id;
  u16 type;
  u8 group;
  Gender gender : 4;
  u8 is_plural_trainers : 4;
  u8 trainer_type_grammar;

  u8 money_calculation_coefficient;
  ItemId items[4];

  u32 message_archive_id;
  u16 win_string_id;
  u16 lose_string_id;

  String* name;
  String* title_name;
  TrainerAppearance* appearance;
};
} // namespace battle
