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

#include "battle/config.h"
#include "battle/manager.h"
#include "common.h"
#include "core/game_time_manager.h"
#include "data/move.h"
#include "data/pokemon.h"
#include "hack/cheat_code.h"
#include "hack/cheat_code_manager.h"
#include "menu/plugin_menu.h"
#include "overworld/encounter.h"
#include "overworld/field_move.h"
#include "overworld/model_manager.h"
#include "overworld/renderer.h"
#include "overworld/weather_manager.h"
#include "savedata/savedata.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "system/sound.h"

String String::s_tmp;
c16 String::s_buffer[128];

void MainMenu(menu::PluginMenu& menu, void* args) {
  extern void TestMenu(menu::PluginMenu& menu, void* args);
  menu.Add("Renderer", overworld::Renderer::LoadMenu)
      .Add("Encounter", overworld::Encounter::LoadMenu)
      .Add("Field Move", overworld::FieldMove_LoadMenu)
      .Add("Battle Teams", battle::Manager::LoadMenu)
      .Add("Pokemon Data", data::Pokemon::LoadMenu)
      .Add("Move Data", data::Move::LoadMenu)
      .Add("Battle Config", battle::Config::LoadMenu)
      .Add("Camera", overworld::StereoCamera::LoadMenu)
      .Add("Overworld", overworld::ModelManager::LoadMenu)
      .Add("Weather", overworld::WeatherManager::LoadMenu)
      .Add("Time", GameTimeManager::LoadMenu)
      .Add("SaveData", savedata::SaveData::LoadMenu)
      .Add("Sound", Sound::LoadMenu)
      .Add("Test", TestMenu);
}

void Initialize() {
  menu::PluginMenu::GetInstance().EnterSubMenu(MainMenu, nullptr);

  File::MountSdmc();

  // Disables the keyboard's "No Good Word" filter to allow prohibited words,
  // phone numbers, etc.
  WRITE(u32, 0x003A47C0, 0xE3A00000);
  ARM_RET(0x003A47C4);

  // Disable material shader
  // ARM_RET(0x003989B0);
  // ARM_RET(0x003881EC);

  // Disables in-game user inputs to prevent any character actions while the
  // menu is active.
  Device::SetupHooks();

  CheatCodeManager& man = CheatCodeManager::GetInstance();
  man.Add(CheatCodeId::kNoclip, overworld::ModelManager::Noclip);
  man.Add(CheatCodeId::kSwarmMod, overworld::ModelManager::SwarmMod);
}

// Performs logic update and rendering for both screens.
// Called once per frame.
void OnFrame() {
  Graphics& graphics = Graphics::GetInstance();
  menu::PluginMenu& menu = menu::PluginMenu::GetInstance();

  menu.Update();

  CheatCodeManager::GetInstance().Update();

  if (!menu.IsOpened()) return;

  void* top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    Graphics::EnableScissor(0, 0, 400, 240);
    Graphics::BeginRender(top_buffer);
    menu.DrawTop();
    Graphics::DisableScissor();
  }

  void* bottom_buffer = graphics.GetFramebuffer(Screen::kBottom);
  if (graphics.BindFramebuffer(bottom_buffer)) {
    Graphics::EnableScissor(0, 0, 320, 240);
    Graphics::BeginRender(bottom_buffer);
    menu.DrawBottom();
    Graphics::DisableScissor();
  }
}