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

String String::s_tmp;
c16 String::s_buffer[128];

static s32 s_game_speed = 2;
static u32 s_frame_count = 0;

s32 OnUpdateFrame(uptr addr) {
  s32 res = 0;
  s_frame_count++;

  if (s_game_speed >= 1) {
    for (s32 i = 0; i < s_game_speed; i++) {
      res = HookManager::GetInstance().Get(HookID::kOnUpdateFrame)->CallOriginal
          <s32>(addr);
    }
    return res;
  }

  if (s_game_speed < 0) {
    s32 divider = -s_game_speed;
    if (s_frame_count % divider == 0) {
      return HookManager::GetInstance().Get(HookID::kOnUpdateFrame)->
                                        CallOriginal<s32>(addr);
    }
    return 1;
  }

  return HookManager::GetInstance().Get(HookID::kOnUpdateFrame)->CallOriginal<
    s32>(addr);
}

void LayoutMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Text Box", layout::TextBox::LoadMenu)
      .Add("Picture", layout::Picture::LoadMenu);
}

void GlobalDataMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Global Pokemon Data", data::Pokemon::LoadMenu)
      .Add("Global Move Data", data::Move::LoadMenu)
      .Add("Global Time", GameTimeManager::LoadMenu)
      .Add("Date Time (ms)", *(s64*)ADDRESS_DATE_TIME);
}

void OverworldMenu(menu::PluginMenu& menu, void* args) {
  overworld::MapManager& man = overworld::MapManager::GetInstance();
  menu.Add("Reload Map", man.GetMapId())
      .Add("Map Tile", overworld::Tile::LoadMenu)
      .Add("Renderer", overworld::Renderer::LoadMenu)
      .Add("Camera", overworld::StereoCamera::LoadMenu)
      .Add("Model", overworld::ModelManager::LoadMenu)
      .Add("Encounter", overworld::Encounter::LoadMenu);
  menu.AddSeparator();
  overworld::WeatherManager::LoadMenu(menu, args);
  menu.AddSeparator();
  overworld::FieldMove_LoadMenu(menu, args);
}

void EnableInstantEggHatch(void*) {
  // Skip check for remaining hatch steps
  // (cf. savedata/pokemon_core_data.h)
  WRITE(u32, 0x00715EF0, 0xEA000007);
}

void EnableInstantEggGeneration(void*) {
  WRITE(u32, 0x00711364, 0xE1A00000);
}

void EnableInstantMaxExp(void*) {
  WRITE(u32, 0x00465A34, 0x15824004);
  WRITE(u32, 0x00465A54, 0x158240F4);
}

void DayCareMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Instant Egg Hatch", EnableInstantEggHatch)
      .Add("Instant Egg Generation", EnableInstantEggGeneration)
      .Add("Instant Max Exp", EnableInstantMaxExp);
}

void TopMenu(menu::PluginMenu& menu, void* args) {
  overworld::MapManager& man = overworld::MapManager::GetInstance();

  menu.Add("Game Speed", s_game_speed)
      .Add("Global Data", GlobalDataMenu)
      .Add("Save Data", savedata::SaveData::LoadMenu)
      .Add("Overworld", OverworldMenu)
      .Add("Layout", LayoutMenu)
      .Add("Battle", battle::Manager::LoadMenu)
      .Add("PSS", PssManager::LoadMenu)
      .Add("Day Care", DayCareMenu)
      .Add("Sound", Sound::LoadMenu)
      .Add("Plugin Theme", menu::Theme::LoadMenu);
}

u32 OnSaveGameData(u32 fs) {
  u32 result = HookManager::GetInstance().Get(HookID::kOnSaveGameData)->
                                          CallOriginal<u32>(fs);
  menu::LogMenu::GetInstance().Add(u"Saving...");
  return result;
}

uptr OnLoadCroFile(uptr man, uptr heap, const char* filename, u32* size) {
  uptr buffer = HookManager::GetInstance().Get(HookID::kOnLoadCroFile)->
                                           CallOriginal<uptr>(
                                               man, heap, filename, size);
  menu::LogMenu::GetInstance().Add(u"%s loaded at 0x%X (0x%X bytes)",
                                   filename, buffer, *size);
  return buffer;
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

  HookManager& hook_manager = HookManager::GetInstance();
  hook_manager.Add(HookID::kOnLoadCroFile, 0x00110E2C,
                   (uptr)OnLoadCroFile);
  hook_manager.Add(HookID::kOnSaveGameData, 0x0036C47C,
                   (uptr)OnSaveGameData);
  hook_manager.Add(HookID::kOnUpdateFrame, 0x0011EEA4,
                   (uptr)OnUpdateFrame);
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