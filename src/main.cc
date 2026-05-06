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
#include "hack/hook_manager.h"
#include "layout/picture.h"
#include "menu/plugin_menu.h"
#include "overworld/encounter.h"
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

extern void TestMenu(menu::PluginMenu& menu, void* args);

namespace overworld {
extern void FieldMove_LoadMenu(menu::PluginMenu& menu, void* args);
}

namespace layout {
extern void OnDrawPicture(Picture* picture, u32 p0, u32 p1, u32 p2);
}

void EnablePsychedelicVision(void*) {
  HookManager::GetInstance().Add(HookID::kOnDrawPicture,
                                 ADDRESS_LAYOUT_DRAW_PICTURE,
                                 (uptr)layout::OnDrawPicture);
}

void MainMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Psychedelic Vision", EnablePsychedelicVision)
      .Add("Renderer", overworld::Renderer::LoadMenu)
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
      .Add("Savedata", savedata::SaveData::LoadMenu)
      .Add("Test", TestMenu)
      .Add("Sound", Sound::LoadMenu);
}

void SetupCheatCodes() {
  CheatCodeManager& man = CheatCodeManager::GetInstance();
  man.Add(CheatCodeId::kNoclip, overworld::ModelManager::Noclip);
  man.Add(CheatCodeId::kSwarmMod, overworld::ModelManager::SwarmMod);
}

void ApplyPatches() {
  // Disables the keyboard's "No Good Word" filter to allow prohibited words,
  // phone numbers, etc.
  WRITE(u32, 0x003A47C0, 0xE3A00000);
  ARM_RET(0x003A47C4);

  // Remove H3D MODELS
  // ARM_RET(0x0038B928);

  // Disable material shader
  // ARM_RET(0x003989B0);
  // ARM_RET(0x003881EC);

  // Disable outline
  // WRITE(u32, 0x0038BE34, 0xE12FFF1E);

  // Skip script
  // WRITE(u32, 0x003DC3DC, 0xE3A00001);
  // WRITE(u32, 0x003DC3E0, 0xE12FFF1E);

  // Disables in-game user inputs to prevent any character actions while the
  // menu is active.
  Device::SetupHooks();
  overworld::StereoCamera::SetupHooks();
}

// Performs logic update and rendering for both screens.
// Called once per frame.
extern "C" void OnFrame() {
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


extern "C" void Initialize() {
  File::MountSdmc();
  menu::PluginMenu::GetInstance().EnterSubMenu(MainMenu, nullptr);
  ApplyPatches();
  SetupCheatCodes();
}

#include <CTRPluginFramework.hpp>

namespace CTRPluginFramework {
int main() {
  Initialize();
  Hook hook;
  hook.InitializeForMitm(0x00122938, (uptr)OnFrame);
  hook.Enable();
  return 0;
}
} // namespace CTRPluginFramework