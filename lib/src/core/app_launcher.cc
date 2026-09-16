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

#include "core/patch/app_launcher.h"
#include "core/hook_manager.h"
#include "core/native/game_manager.h"
#include "pokemon/native/data_accessor.h"
#include "savedata/native/bag_manager.h"
#include "savedata/native/battle_box.h"
#include "savedata/native/box_manager.h"
#include "savedata/native/item_manager.h"
#include "savedata/native/misc.h"
#include "savedata/native/pokemon_box.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/trainer_status.h"
#include "ui/main_application.h"

namespace core {

namespace {
/// Layout of the game's app request, as reached from the CallApp hook.
constexpr uptr kOffsetAppId = 28;
constexpr uptr kOffsetInput = 0x20;
constexpr uptr kOffsetCallback = 0x2C;
constexpr uptr kOffsetTownMapCallback = 0x30;
/// The move the Move Tutor teaches when opened from the menu.
constexpr MoveId kTutorMove = MoveId::kDragonAscent;
} // namespace

void AppLauncher::Initialize() {
  // Alloc 0x100 bytes for app hook
  WRITE32(address::kOverworldMenuAppAllocSize, 0xE3A01C01); // Overworld Menu
  WRITE32(address::kTopMenuAppAllocSize, 0xE3A01C01); // Top Menu
  HookManager::Initialize(HookId::kCallApp, sys::address::kCallApp,
                          (uptr)CallAppHook);
  HookManager::Initialize(HookId::kCheckAppRequest,
                          sys::address::kCheckAppRequest,
                          (uptr)CheckAppRequestHook, false);
}

void AppLauncher::TriggerApp(AppId id) {
  app_id = id;
  open_app = true;
  ui::MainApplication::GetInstance().ForceClose();
}

void AppLauncher::DoFly() {
  GetInstance().TriggerApp(AppId::kTownMap);
}

bool AppLauncher::CheckAppRequestHook(uptr menu, u32 id) {
  auto& ctx = GetInstance();
  if (ctx.open_app) {
    WRITE32(menu + 4 * (5 >> 5) + 0xB0, 1 << (5 % 32));
  }
  return HookManager::Call<bool>(HookId::kCheckAppRequest, menu, id);
}

void AppLauncher::MoveDeleterCallback(uptr* data, GameManager* manager) {
  auto* input = (MoveAppInput*)data[1];
  if (input->delete_move) {
    input->pokemon->accessor->Decrypt();
    input->pokemon->core->moves[input->move_index] = MoveId::kNone;
    input->pokemon->accessor->Encrypt();
  }
}

void AppLauncher::MoveTutorCallback(uptr* data, GameManager* manager) {
  auto* input = (MoveAppInput*)data[1];
  input->pokemon->accessor->Decrypt();
  input->pokemon->core->moves[input->move_index] = input->move_id;
  input->pokemon->accessor->Encrypt();
}

void AppLauncher::TownMapCallback(uptr* data, GameManager* manager) {
  auto* input = (TownMapAppInput*)data[1];
  input->result = 2;
  input->pokemon_index = 0;
  ((void(*)(uptr*, void*))address::kTownMapCallback)(data, manager);
}

void AppLauncher::CallAppHook(uptr self, GameManager* manager) {
  auto& ctx = GetInstance();

  u8 choice = READ8(self + kOffsetAppId);
  if (choice != 0 || !ctx.open_app) {
    HookManager::Call<void>(HookId::kCallApp, self, manager);
    return;
  }

  WRITE8(self + kOffsetAppId, static_cast<u8>(ctx.app_id));
  switch (ctx.app_id) {
    case AppId::kMoveTutor: {
      auto* input = (MoveAppInput*)READ32(self + kOffsetInput);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      input->move_id = kTutorMove;
      WRITE32(self + kOffsetCallback, (uptr)MoveTutorCallback);
      break;
    }
    case AppId::kMoveDeleter: {
      auto* input = (MoveAppInput*)READ32(self + kOffsetInput);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      WRITE32(self + kOffsetCallback, (uptr)MoveDeleterCallback);
      break;
    }
    case AppId::kMoveReminder: {
      auto* input = (MoveAppInput*)READ32(self + kOffsetInput);
      input->pokemon = savedata::PokemonTeam::GetInstance().pokemons[0];
      break;
    }
    case AppId::kBox: {
      auto* input = (BoxAppInput*)READ32(self + kOffsetInput);
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
      break;
    }
    case AppId::kTownMap: {
      auto* input = (TownMapAppInput*)READ32(self + kOffsetInput);
      input->is_fly_mode = true;
      WRITE32(self + kOffsetTownMapCallback, (uptr)TownMapCallback);
      break;
    }
  }

  ctx.open_app = false;

  HookManager::Call<void>(HookId::kCallApp, self, manager);
}

} // namespace core
