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
#include "feature_nuzlocke.h"
#include "hook_manager.h"
#include "game/manager.h"
#include "game/process_manager.h"
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
#include "ui/log_application.h"
#include "ui/main_application.h"

namespace feature {
class GameApp {
  MAKE_SINGLETON(GameApp)
  bool open_app = false;
  u32 app_id = 0;

public:
  STATIC_INLINE void Initialize() {
    // Overworld Menu
    WRITE(vu32, 0x003D6D18, 0xE3A01C01); // Alloc 0x100 bytes for app hook
    // Top Menu
    WRITE(vu32, 0x003D81D4, 0xE3A01C01);
    HookManager::Initialize(HookID::kCallApp, ADDRESS_CALL_APP,
                            (uptr)CallAppHook);
    HookManager::Initialize(HookID::kCheckAppRequest, ADDRESS_CHECK_APP_REQUEST,
                            (uptr)CheckAppRequestHook, false);
  }

  static void OnEnterAppStatus() {
  }

  static void OnUpdateAppStatus() {
    auto& controller = Controller::GetInstance();
    static u32 index = 0;
    static u32 get_stats[] = {
        ADDRESS_POKEMON_GET_STATS, ADDRESS_POKEMON_GET_EVS,
        ADDRESS_POKEMON_GET_IVS};
    static u32 pokemon_index = 0;
    u32 max = savedata::PokemonTeam::GetInstance().count;
    if (controller.IsKeyPressed(Key::kDown)) {
      pokemon_index = (pokemon_index + 1) % max;
    }
    if (controller.IsKeyPressed(Key::kUp)) {
      pokemon_index = (pokemon_index - 1 + max) % max;
    }
    if (controller.IsKeyPressed(Key::kR)) {
      index = (index + 1) % 3;
      WRITE(vu32, ADDRESS_APP_STATUS_GET_STAT_TRAMPOLINE, get_stats[index]);
      game::BaseProcess* process = game::ProcessManager::GetInstance().
          GetCurrentProcess();
      ((void(*)(void*, u32, bool))ADDRESS_APP_STATUS_UPDATE_POKEMON)(process,
        pokemon_index, false);
    }
    if (controller.IsKeyPressed(Key::kL)) {
      index = (index - 1 + 3) % 3;
      WRITE(vu32, ADDRESS_APP_STATUS_GET_STAT_TRAMPOLINE, get_stats[index]);
      game::BaseProcess* process = game::ProcessManager::GetInstance().
          GetCurrentProcess();
      ((void(*)(void*, u32, bool))ADDRESS_APP_STATUS_UPDATE_POKEMON)(process,
        pokemon_index, false);
    }
  }

  static void OnEnterOverworld() {
    // Simulate a button press
    WRITE(vu32, 0x00715C48, 0xE1A00000);
    HookManager::ForceEnable(HookID::kCheckAppRequest);
    HookManager::ForceEnable(HookID::kScriptCreatePokemon);
    HookManager::ForceEnable(HookID::kEncounterSetPokemon);
    HookManager::ForceEnable(HookID::kSetCulling);
  }

  void TriggerApp(u32 id) {
    app_id = id;
    open_app = true;
    ui::MainApplication::GetInstance().ForceClose();
  }

  static void DoFly() {
    GetInstance().TriggerApp(15);
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

  struct TownMapInput {
    bool is_fly_mode;
    u8 _0[0x20 - 1];
    u8 result;
    u8 _1;
    u16 map_id;
    u16 pokemon_index;
  };

  static bool CheckAppRequestHook(uptr menu, u32 id) {
    auto& ctx = GetInstance();
    if (ctx.open_app) {
      *(u32*)(menu + 4 * (5 >> 5) + 0xB0) = 1 << (5 % 32);
    }

    return HookManager::Call<bool>(HookID::kCheckAppRequest, menu, id);
  }

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

  static void TownMapCallback(uptr* data, game::Manager* manager) {
    auto* input = (TownMapInput*)data[1];
    input->result = 2;
    input->pokemon_index = 0;
    ((void(*)(uptr*, void*))0x003EF898)(data, manager);
  }

  static void CallAppHook(uptr self, game::Manager* manager) {
    auto& controller = Controller::GetInstance();
    auto& ctx = GetInstance();

    u8& choice = READ(u8, self + 28);
    if (choice != 0 || !ctx.open_app) {
      HookManager::Call<void>(HookID::kCallApp, self, manager);
      return;
    }

    // Call Move Tutor
    if (ctx.app_id == 7) {
      choice = 7;
      auto* input = (MoveInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      input->move_id = 620;
      WRITE(u32, self + 0x2C, (uptr)MoveTutorCallback);
    }

    // Call Move Deleter
    if (ctx.app_id == 8) {
      choice = 8;
      auto* input = (MoveInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      WRITE(u32, self + 0x2C, (uptr)MoveDeleterCallback);
    }

    // Call Move Reminder
    if (ctx.app_id == 9) {
      choice = 9;
      auto* input = (MoveInput*)READ(u32, self+0x20);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
    }

    // Call Box
    if (ctx.app_id == 17) {
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

    // Call Townmap
    if (ctx.app_id == 15) {
      choice = 15;
      TownMapInput* data = (TownMapInput*)READ(u32, self+0x20);
      data->is_fly_mode = true;
      WRITE(u32, self + 0x30, (uptr)TownMapCallback);
    }

    ctx.open_app = false;

    HookManager::Call<void>(HookID::kCallApp, self, manager);
  }
};
} // namespace feature