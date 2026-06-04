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

#include "savedata/pss.h"

struct PssData {
  savedata::PssUserData user_data;

  u32 nice_received : 1;
  u32 nice_count_blocked : 1;
  u32 nice_display_blocked : 1;
  u32 birthday_count_blocked : 1;
  u32 birthday_display_blocked : 1;
  u32 is_locked : 1;
  u32 is_online : 1;
  u32 message_displayed : 1;
  u32 affection_anim_done : 1;
  u32 active_opower : 8;
  u32  : 15;

  struct {
    void* next;
    void* previous;
  } hash_node, node;
};

class PssManager {
public:
  STATIC_INLINE PssManager& GetInstance() {
    return game::GameManager::GetInstance().GetPssManager();
  }

  INLINE savedata::PssProfilePayload& GetMyProfile() {
    return self_data_.user_data.datagram.profile;
  }

private:
  PssData self_data_;
};