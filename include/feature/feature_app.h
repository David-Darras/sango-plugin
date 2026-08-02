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
#include "feature_device.h"
#include "hook_manager.h"
#include "../game/renderer/app_layout_manager.h"
#include "game/manager.h"
#include "game/process_manager.h"
#include "game/constant/ability.h"
#include "game/renderer/text_box.h"
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
#include "ui/main_application.h"

namespace feature {
class GameApp {
  MAKE_SINGLETON(GameApp)
  bool open_app = false;
  u32 app_id = 0;

public:
  STATIC_INLINE void Initialize() {
    // Alloc 0x100 bytes for app hook
    WRITE(vu32, 0x003D6D18, 0xE3A01C01); // Overworld Menu
    WRITE(vu32, 0x003D81D4, 0xE3A01C01); // Top Menu
    HookManager::Initialize(HookID::kCallApp, ADDRESS_CALL_APP,
                            (uptr)CallAppHook);
    HookManager::Initialize(HookID::kCheckAppRequest, ADDRESS_CHECK_APP_REQUEST,
                            (uptr)CheckAppRequestHook, false);
  }

  // static void AppStatusSetupGraphicsHook(uptr self,
  //                                        savedata::PokemonParam* pokemon) {
  //   HookManager::Call<void>(HookID::kAppStatusSetupGraphics, self, pokemon);
  //   auto& manager = *(AppLayoutManager*)(READ(uptr, self + 8 + 16));
  //   if (!feature::DeviceState::GetInstance().use_redirection) {
  //     return;
  //   }
  //
  //   auto& state = GetInstance().app_status.state;
  //   static const char* PREFIX[3] = {"", "IV ", "EV "};
  //   pokemon->accessor->Decrypt();
  //   auto& core = *pokemon->core;
  //   auto& runtime = *pokemon->runtime;
  //   const u16 VALUES[3][6] = {
  //       {runtime.hp, runtime.attack, runtime.defense, runtime.special_attack,
  //        runtime.special_defense, runtime.speed},
  //       {(u16)core.iv_hp, (u16)core.iv_attack, (u16)core.iv_defense,
  //        (u16)core.iv_special_attack,
  //        (u16)core.iv_special_defense, (u16)core.iv_speed},
  //       {(u16)core.ev_hp, (u16)core.ev_attack, (u16)core.ev_defense,
  //        (u16)core.ev_special_attack,
  //        (u16)core.ev_special_defense, (u16)core.ev_speed},
  //   };
  //   pokemon->accessor->Encrypt();
  //
  //   const char* prefix = PREFIX[state];
  //   const u16* values = VALUES[state];
  //
  //   static const u8 LABEL_PANES[] = {0, 8, 12, 16, 20, 24};
  //   static const u8 VALUE_PANES[] = {4, 10, 14, 18, 22, 26};
  //   static const u8 TEXTS[] = {17, 19, 21, 23, 25, 27};
  //   static const c16* LABELS[] = {
  //       u"%sHP", u"%sAttack", u"%sDefense", u"%sSp. Atk", u"%sSp. Def",
  //       u"%sSpeed"
  //   };
  //
  //   auto& cursor = GetInstance().app_status.cursor;
  //   for (u32 i = 0; i < 6; i++) {
  //     Color8 top(255, 255, 255, 255);
  //     Color8 bottom(255, 255, 255, 255);
  //     if (cursor == i) {
  //       top = Color8(255, 0, 0, 255);
  //       bottom = Color8(255, 255, 0, 255);
  //     }
  //     manager.SetTextBoxStringValue(0, LABEL_PANES[i], LABELS[i], prefix);
  //     manager.SetTextBoxColor(0, LABEL_PANES[i], &top, &bottom);
  //     manager.SetTextBoxIntegerValue(0, VALUE_PANES[i], TEXTS[i], values[i], 3);
  //     manager.SetTextBoxColor(0, VALUE_PANES[i], &top, &bottom);
  //   }
  //
  //   if (cursor == 6) {
  //     Color8 top = Color8(255, 0, 0, 255);
  //     Color8 bottom = Color8(255, 255, 0, 255);
  //     manager.SetTextBoxColor(0, 38, &top, &bottom);
  //     manager.SetTextBoxColor(0, 40, &top, &bottom);
  //   } else {
  //     Color8 top = Color8(255, 255, 255, 255);
  //     Color8 bottom = Color8(255, 255, 255, 255);
  //     manager.SetTextBoxColor(0, 38, &top, &bottom);
  //     manager.SetTextBoxColor(0, 40, &top, &bottom);
  //   }
  //
  //   manager.Hide(0, 2); // don't show /
  //   manager.Hide(0, 6); // don't show max hp
  // }
  //
  // static void PatchAppStatus() {
  //   auto& controller = Controller::GetInstance();
  //   u32 max = savedata::PokemonTeam::GetInstance().count;
  //   bool& is_edition = feature::DeviceState::GetInstance().use_redirection;
  //   u8& pokemon_index = GetInstance().app_status.pokemon_index;
  //   u8& cursor = GetInstance().app_status.cursor;
  //   bool refresh = false;
  //   if (controller.IsKeyRepeated(Key::kY)) {
  //     is_edition ^= true;
  //     refresh = true;
  //   }
  //   if (!is_edition) {
  //     if (controller.IsKeyRepeated(Key::kDown)) {
  //       pokemon_index = (pokemon_index + 1) % max;
  //     }
  //     if (controller.IsKeyRepeated(Key::kUp)) {
  //       pokemon_index = (pokemon_index - 1 + max) % max;
  //     }
  //     return;
  //   }
  //   u8& state = GetInstance().app_status.state;
  //   if (controller.IsKeyRepeated(Key::kR)) {
  //     state = (state + 1) % 3;
  //     refresh = true;
  //   }
  //   if (controller.IsKeyRepeated(Key::kL)) {
  //     state = (state - 1 + 3) % 3;
  //     refresh = true;
  //   }
  //   s32 offset = 0;
  //   if (controller.IsKeyRepeated(Key::kRight)) {
  //     offset = 1;
  //     refresh = true;
  //   }
  //   if (controller.IsKeyRepeated(Key::kLeft)) {
  //     offset = -1;
  //     refresh = true;
  //   }
  //   if (controller.IsKeyRepeated(Key::kUp)) {
  //     cursor = (cursor - 1 + 7) % 7;
  //     refresh = true;
  //   }
  //   if (controller.IsKeyRepeated(Key::kDown)) {
  //     cursor = (cursor + 1) % 7;
  //     refresh = true;
  //   }
  //
  //   if (offset != 0) {
  //     auto* pokemon = savedata::PokemonTeam::GetInstance().pokemons[
  //       pokemon_index];
  //     pokemon->accessor->Decrypt();
  //     switch (cursor) {
  //       case 6:
  //         pokemon->core->ability =
  //             (pokemon->core->ability + offset) % ABILITY_COUNT;
  //         break;
  //     }
  //     if (state == 1) {
  //       switch (cursor) {
  //         case 0:
  //           pokemon->core->iv_hp = pokemon->core->iv_hp + offset;
  //           break;
  //         case 1:
  //           pokemon->core->iv_attack = pokemon->core->iv_attack + offset;
  //           break;
  //         case 2:
  //           pokemon->core->iv_defense = pokemon->core->iv_defense + offset;
  //           break;
  //         case 3:
  //           pokemon->core->iv_special_attack =
  //               pokemon->core->iv_special_attack + offset;
  //           break;
  //         case 4:
  //           pokemon->core->iv_special_defense =
  //               pokemon->core->iv_special_defense + offset;
  //           break;
  //         case 5:
  //           pokemon->core->iv_speed = pokemon->core->iv_speed + offset;
  //           break;
  //       }
  //     } else if (state == 2) {
  //       switch (cursor) {
  //         case 0:
  //           pokemon->core->ev_hp = pokemon->core->ev_hp + offset;
  //           break;
  //         case 1:
  //           pokemon->core->ev_attack = pokemon->core->ev_attack + offset;
  //           break;
  //         case 2:
  //           pokemon->core->ev_defense = pokemon->core->ev_defense + offset;
  //           break;
  //         case 3:
  //           pokemon->core->ev_special_attack =
  //               pokemon->core->ev_special_attack + offset;
  //           break;
  //         case 4:
  //           pokemon->core->ev_special_defense =
  //               pokemon->core->ev_special_defense + offset;
  //           break;
  //         case 5:
  //           pokemon->core->ev_speed = pokemon->core->ev_speed + offset;
  //           break;
  //       }
  //     }
  //     pokemon->accessor->Encrypt();
  //     pokemon->UpdateRuntimeData();
  //     refresh = true;
  //   }
  //
  //   if (refresh) {
  //     game::BaseProcess* process = game::ProcessManager::GetInstance().
  //         GetCurrentProcess();
  //     bool& update_pokemon = *(bool*)(*(uptr*)((uptr)process + 104) + 284);
  //     update_pokemon = false;
  //     ((void(*)(void*, u32, bool))ADDRESS_APP_STATUS_UPDATE_POKEMON)(process,
  //       pokemon_index, false);
  //     update_pokemon = true;
  //   }
  // }

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