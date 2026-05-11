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
#include "core/pss_manager.h"
#include "data/move.h"
#include "data/pokemon.h"
#include "hack/cheat_code.h"
#include "hack/cheat_code_manager.h"
#include "layout/picture.h"
#include "layout/text_box.h"
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

String String::s_tmp;
c16 String::s_buffer[128];

void LayoutMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Text Box", layout::TextBox::LoadMenu)
      .Add("Picture", layout::Picture::LoadMenu);
}

void GlobalDataMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Global Pokemon Data", data::Pokemon::LoadMenu)
      .Add("Global Move Data", data::Move::LoadMenu)
      .Add("Global Time", GameTimeManager::LoadMenu);
}

void OverworldMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Map Tile", overworld::Tile::LoadMenu)
      .Add("Renderer", overworld::Renderer::LoadMenu)
      .Add("Camera", overworld::StereoCamera::LoadMenu)
      .Add("Model", overworld::ModelManager::LoadMenu)
      .Add("Encounter", overworld::Encounter::LoadMenu);
  menu.AddSeparator();
  overworld::WeatherManager::LoadMenu(menu, args);
  menu.AddSeparator();
  overworld::FieldMove_LoadMenu(menu, args);
}

static u8 s_game_speed = 2;

// See : https://gbatemp.net/threads/how-to-change-game-speed-independently-of-fps-example-with-pokemon-oras.680385/
void UpdateGameSpeed(void*) {
  static u32 opcodes[] = {
      0xE3A06001, 0xE3A0C000 | s_game_speed, 0xE58FC010, 0xE59FC00C, 0xE25CC001,
      0x0A000075,
      0xE58FC000, 0xEA00000E, 0xFFFFFFFF
  };
  for (u32 i = 0; i < SIZE(opcodes); ++i) {
    WRITE(u32, 0x0010E36C + i * 4, opcodes[i]);
  }
  WRITE(u32, 0x0010E528, 0xEAFFFF92);
  menu::PluginMenu::GetInstance().ForceClose();
}

void TopMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Global Data", GlobalDataMenu)
      .Add("Save Data", savedata::SaveData::LoadMenu)
      .Add("Overworld", OverworldMenu)
      .Add("Layout", LayoutMenu)
      .Add("Battle", battle::Manager::LoadMenu)
      .Add("PSS", PssManager::LoadMenu)
      .Add("Sound", Sound::LoadMenu)
      .Add("Game Speed", s_game_speed)
      .WithBounds(2, 4)
      .Add("Update Game Speed", UpdateGameSpeed)
      .Add("Plugin Theme", menu::Theme::LoadMenu);
}

void Initialize() {
  menu::PluginMenu::GetInstance().EnterSubMenu(TopMenu, nullptr);

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