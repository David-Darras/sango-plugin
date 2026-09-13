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

#include "core/patch/process_patch.h"
#include "battle/patch/battle.h"
#include "core/patch/app_launcher.h"
#include "core/hook_manager.h"
#include "overworld/patch/field.h"
#include "ui/patch/app_status.h"
#include "ui/patch/keyboard_patch.h"
#include "ui/patch/new_game.h"
#include "ui/patch/starter_choice.h"
#include "ui/patch/title_screen.h"
#include "core/native/process_manager.h"

namespace core {

void ProcessPatch::Initialize() {
  HookManager::Initialize(HookId::kMainProcessLoop, sys::address::kMainProcessLoop,
                          (uptr)MainProcessLoopHook);
}

u32 ProcessPatch::MainProcessLoopHook(ProcessManager* manager) {
  manager->Patch(OnLoad, OnUpdate);
  return HookManager::Call<u32>(HookId::kMainProcessLoop, manager);
}

void ProcessPatch::OnUpdate(uptr vtable) {
  ui::NewGame::OnProcessUpdate(vtable);
  switch (vtable) {
    case ui::address::kAppStatusVtable:
      ui::AppStatus::PatchUpdate();
      break;
    case battle::address::kVtable:
      battle::Battle::PatchUpdate();
      break;
    default:
      break;
  }
}

void ProcessPatch::OnLoad(uptr vtable) {
  auto& feat = GetInstance();
  if (feat.on_process_load != nullptr) feat.on_process_load(vtable);
  ui::NewGame::OnProcessLoad(vtable);

  if (vtable != overworld::address::kVtable) {
    HookManager::Clear(HookId::kGetEncounterPokemon);
  }
  if (vtable != ui::address::kKeyboardVtable) {
    ui::KeyboardPatch::GetInstance().is_opened = false;
  }

  switch (vtable) {
    case address::kTitleScreenVtable:
      ui::TitleScreen::PatchLoad();
      break;
    case battle::address::kVtable:
      battle::Battle::PatchLoad();
      break;
    case overworld::address::kVtable:
      overworld::Field::PatchLoad();
      break;
    case ui::address::kAppStatusVtable:
      ui::AppStatus::PatchLoad();
      break;
    case ui::address::kKeyboardVtable:
      ui::KeyboardPatch::PatchLoad();
      break;
    case address::kSelectStarterVtable:
      ui::StarterChoice::PatchLoad();
      break;
    default:
      break;
  }
}

} // namespace core
