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
#include "archive.h"
#include "common.h"
#include "hook_manager.h"
#include "game/constant/move.h"
#include "game/global_data/evolve.h"
#include "game/global_data/move.h"

namespace feature {
class GameTextManager {
  MAKE_SINGLETON(GameTextManager)
public:
  struct GameText {
    uptr vtable;
    uptr heap;
    u32* archive;
    u32 _0;
    u32 file_id;
    u32 language;
    uptr _1;
    uptr _2;
  };

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGameTextManagerGetText,
                            ADDRESS_GAME_TEXT_MANAGER_GET_TEXT,
                            (uptr)GetGameText);
    HookManager::Initialize(HookID::kGetMoveName,
                            ADDRESS_GET_MOVE_NAME,
                            (uptr)GetMoveName);
    HookManager::Initialize(HookID::kLoadMoveData,
                            ADDRESS_LOAD_MOVE_DATA,
                            (uptr)LoadMoveData);
  }

  static u32 LoadMoveData(uptr self, u16 move_id) {
    u32 id = move_id;
    if (id == MOVE_XXX) id = 1;
    u32 result = HookManager::Call<u32>(HookID::kLoadMoveData, self, id);
    if (move_id == MOVE_XXX) {
      auto& move = *(global_data::Move*)(READ32(self + 8));
      move.power = 200;
      move.type = 3;
      move.accuracy = 100;
      WRITE16(self + 4, move_id);
    }
    return result;
  }

  static u32 GetMoveName(uptr self, u16 move, String* output) {
    u32 result = HookManager::Call<u32>(HookID::kGetMoveName, self, move,
                                        output);
    if (move == MOVE_XXX) {
      output->buffer[0] = 0x40;
      output->buffer[1] = 0x40;
      output->buffer[2] = 0x40;
      output->buffer[3] = 0;
      output->size = 3;
    }
    return result;
  }

  static bool IsGameText(GameText* text) {
    u32* archive_table = (u32*)ADDRESS_ARCHIVE_FILENAME_TABLE;
    for (u32 i = ARCHIVE_GAME_TEXT_JAPANESE;
         i <= ARCHIVE_GAME_TEXT_KOREAN;
         i++) {
      if (text->archive[12] == archive_table[i]) {
        return true;
      }
    }
    return false;
  }

  STATIC_INLINE bool IsMoveName(GameText* text) {
    return text->file_id == 14 || text->file_id == 15;
  }

  STATIC_INLINE bool IsMoveDescription(GameText* text) {
    return text->file_id == 16;
  }

  static u32 GetGameText(GameText* text, u32 text_id, String* output) {
    bool update = IsGameText(text) && (
                    IsMoveName(text) || IsMoveDescription(text))
                  && text_id == MOVE_XXX;
    if (update) text_id = 1;
    u32 result = HookManager::Call<u32>(HookID::kGameTextManagerGetText, text,
                                        text_id,
                                        output);
    if (update) {
      output->buffer[0] = 0x40;
      output->buffer[1] = 0x40;
      output->buffer[2] = 0x40;
      output->buffer[3] = 0;
      output->size = 3;
    }
    return result;
  }
};
}