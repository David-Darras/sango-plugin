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
#include "hook_manager.h"
#include "game/manager.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "system/device.h"

namespace feature {
class ProcessHookContext {
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kCallApp, ADDRESS_CALL_APP,
                            (uptr)CallAppHook);
  }

  struct MoveDeleterInput {
    savedata::PokemonParam* pokemon;
    u16 move_id;
    bool delete_move;
    u8 move_index;
  };

  struct MoveReminderInput {
    savedata::PokemonParam* pokemon;
  };

  static void MoveDeleterCallback(uptr* data, game::Manager* manager) {
    auto* input = (MoveDeleterInput*)data[1];
    if (input->delete_move) {
      input->pokemon->accessor->Decrypt();
      input->pokemon->core->moves[input->move_index] = 0;
      input->pokemon->accessor->Encrypt();
    }
  }

  static void CallAppHook(uptr self, game::Manager* manager) {
    auto& controller = Controller::GetInstance();

    u8& choice = READ(u8, self + 28);
    if (choice != 0) {
      HookManager::Call<void>(HookID::kCallApp, self, manager);
      return;
    }

    // Call Move Deleter
    if (controller.IsKeyDown(Key::kA) && controller.IsKeyDown(Key::kUp)) {
      choice = 8;
      auto* input = (MoveDeleterInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      WRITE(u32, self + 0x2C, (uptr)MoveDeleterCallback);
    }

    // Call Move Reminder
    if (controller.IsKeyDown(Key::kA) && controller.IsKeyDown(Key::kDown)) {
      choice = 9;
      auto* input = (MoveReminderInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
    }

    HookManager::Call<void>(HookID::kCallApp, self, manager);
  }
};
} // namespace feature