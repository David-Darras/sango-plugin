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

#include <cstring>

#include "battle/manager.h"
#include "common.h"
#include "top_menu.h"
#include "core/game_time_manager.h"
#include "core/pokemon_model.h"
#include "core/pss_manager.h"
#include "data/move.h"
#include "data/pokemon.h"
#include "hack/cheat_code.h"
#include "hack/cheat_code_manager.h"
#include "hack/hook_manager.h"
#include "layout/picture.h"
#include "layout/text_box.h"
#include "menu/log_menu.h"
#include "menu/plugin_menu.h"
#include "menu/theme.h"
#include "overworld/encounter.h"
#include "overworld/field_move.h"
#include "overworld/model_manager.h"
#include "overworld/renderer.h"
#include "overworld/tile.h"
#include "overworld/weather_manager.h"
#include "savedata/savedata.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "system/sound.h"
#include "ui/application_manager.h"

void Initialize() {
  menu::PluginMenu::GetInstance().EnterSubMenu(TopMenu, nullptr);

  File::MountSdmc();

  // Disables the keyboard's "No Good Word" filter to allow prohibited words,
  // phone numbers, etc.
  WRITE(u32, 0x003A47C0, 0xE3A00000);
  ARM_RET(0x003A47C4);

  // Disable Shiny
  // WRITE(u8, 0x00168F60, 0);

  // Disable material shader
  // ARM_RET(0x003989B0);
  // ARM_RET(0x003881EC);

  // Disables in-game user inputs to prevent any character actions while the
  // menu is active.
  Device::SetupHooks();

  CheatCodeManager& man = CheatCodeManager::GetInstance();
  man.Add(CheatCodeId::kNoclip, overworld::ModelManager::Noclip);
  man.Add(CheatCodeId::kSwarmMod, overworld::ModelManager::SwarmMod);

  HookManager& hook_manager = HookManager::GetInstance();
  // hook_manager.Add(HookID::kOnLoadCroFile, 0x00110E2C,
  //                  (uptr)OnLoadCroFile);
  // hook_manager.Add(HookID::kOnSaveGameData, 0x0036C47C,
  //                  (uptr)OnSaveGameData);
  // hook_manager.Add(HookID::kOnUpdateFrame, 0x0011EEA4,
  //                  (uptr)OnUpdateFrame);
  hook_manager.Add(HookID::kOnCreatePokemonModel, 0x0046FE44,
                   (uptr)OnCreatePokemonModel);

  ui::ApplicationManager::GetInstance().Push(
      &menu::PluginMenu::GetInstance()
      );
}

void Entrypoint() {
  Graphics& graphics = Graphics::GetInstance();
  Controller& controller = Controller::GetInstance();
  ui::ApplicationManager& manager = ui::ApplicationManager::GetInstance();
  ui::Application* application = manager.GetCurrentApplication();

  application->Update(controller);

  void* top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    Graphics::EnableScissor(0, 0, 400, 240);
    Graphics::BeginRender(top_buffer);
    application->DrawTop(graphics);
    Graphics::DisableScissor();
  }

  void* bottom_buffer = graphics.GetFramebuffer(Screen::kBottom);
  if (graphics.BindFramebuffer(bottom_buffer)) {
    Graphics::EnableScissor(0, 0, 320, 240);
    Graphics::BeginRender(bottom_buffer);
    application->DrawBottom(graphics);
    Graphics::DisableScissor();
  }
}