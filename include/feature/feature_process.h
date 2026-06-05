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
#include "game/savedata/bag_manager.h"
#include "game/savedata/battle_box.h"
#include "game/savedata/box_manager.h"
#include "game/savedata/item_manager.h"
#include "game/savedata/misc.h"
#include "game/savedata/pokemon_box.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/trainer_status.h"
#include "system/device.h"

namespace feature {
class ProcessHookContext {
public:
  STATIC_INLINE void Initialize() {
    WRITE(u32, 0x003D6D18, 0xE3A01C01); // Alloc 0x100 bytes for app hook
    HookManager::Initialize(HookID::kCallApp, ADDRESS_CALL_APP,
                            (uptr)CallAppHook);
  }

  struct MoveInput {
    savedata::PokemonParam* pokemon;
    u16 move_id;
    bool delete_move;
    u8 move_index;
  };

  struct BoxInput {
    savedata::BoxManager* box_manager;
    savedata::PokemonBox* pokemon_box;
    savedata::BattleBox* battle_box;
    savedata::PokemonTeam* team;
    savedata::TrainerStatus* trainer_status;
    savedata::ItemManager* item_manager;
    savedata::BagManager* bag_manager;
    void* bag_data;
    savedata::Misc* misc;

    u32 mode;

    u16 trade_species;
    u8 trade_sex;
    u8 _0;
    u32 trade_level;

    u16 exit_type;
    u8 box_index;
    u8 slot_index;
  };

  static void MoveDeleterCallback(uptr* data, game::Manager* manager) {
    auto* input = (MoveInput*)data[1];
    if (input->delete_move) {
      input->pokemon->accessor->Decrypt();
      input->pokemon->core->moves[input->move_index] = 0;
      input->pokemon->accessor->Encrypt();
    }
  }

  static void MoveTutorCallback(uptr* data, game::Manager* manager) {
    auto* input = (MoveInput*)data[1];
    input->pokemon->accessor->Decrypt();
    input->pokemon->core->moves[input->move_index] = input->move_id;
    input->pokemon->accessor->Encrypt();
  }

  static void CallAppHook(uptr self, game::Manager* manager) {
    auto& controller = Controller::GetInstance();

    u8& choice = READ(u8, self + 28);
    if (choice != 0) {
      HookManager::Call<void>(HookID::kCallApp, self, manager);
      return;
    }

    // Call Move Tutor
    if (controller.IsKeyDown(Key::kA) && controller.IsKeyDown(Key::kRight)) {
      choice = 7;
      auto* input = (MoveInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      input->move_id = 620;
      WRITE(u32, self + 0x2C, (uptr)MoveTutorCallback);
    }

    // Call Move Deleter
    if (controller.IsKeyDown(Key::kA) && controller.IsKeyDown(Key::kUp)) {
      choice = 8;
      auto* input = (MoveInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      WRITE(u32, self + 0x2C, (uptr)MoveDeleterCallback);
    }

    // Call Move Reminder
    if (controller.IsKeyDown(Key::kA) && controller.IsKeyDown(Key::kDown)) {
      choice = 9;
      auto* input = (MoveInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
    }

    // Call Box
    if (controller.IsKeyDown(Key::kA) && controller.IsKeyDown(Key::kLeft)) {
      choice = 17;
      auto* input = (BoxInput*)READ(u32, self+0x20);
      input->box_manager = &savedata::BoxManager::GetInstance();
      input->pokemon_box = &savedata::PokemonBox::GetInstance();
      input->battle_box = &savedata::BattleBox::GetInstance();
      input->team = &savedata::PokemonTeam::GetInstance();
      input->trainer_status = &savedata::TrainerStatus::GetInstance();
      input->item_manager = &savedata::ItemManager::GetInstance();
      input->bag_manager = &savedata::BagManager::GetInstance();
      static u32 bag_data[50];
      input->bag_data = &bag_data;
      input->misc = &savedata::Misc::GetInstance();
      input->mode = 0;
    }

    HookManager::Call<void>(HookID::kCallApp, self, manager);
  }
};
} // namespace feature