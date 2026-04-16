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
#include "common.h"
#include "core/game_time_manager.h"
#include "data/move.h"
#include "data/pokemon.h"
#include "hack/cheat_code.h"
#include "hack/cheat_code_manager.h"
#include "menu/log_menu.h"
#include "menu/plugin_menu.h"
#include "overworld/model_manager.h"
#include "overworld/renderer.h"
#include "overworld/weather_manager.h"
#include "savedata/savedata.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "system/sound.h"

extern void TestMenu(menu::PluginMenu &menu, void *args);

void MainMenu(menu::PluginMenu &menu, void *args) {
  menu.Add("Pokemon Data", data::Pokemon::LoadMenu)
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
  CheatCodeManager &man = CheatCodeManager::GetInstance();
  man.Add(CheatCodeId::kNoclip, overworld::ModelManager::Noclip);
  man.Add(CheatCodeId::kSwarmMod, overworld::ModelManager::SwarmMod);
}

void ApplyPatches() {
  // Disables the keyboard's "No Good Word" filter to allow prohibited words,
  // phone numbers, etc.
  WRITE(u32, 0x003A47C0, 0xE3A00000);
  WRITE(u32, 0x003A47C4, 0xE12FFF1E);

  // Disable material shader
  WRITE(u32, 0x003989B0, 0xE12FFF1E);
  WRITE(u32, 0x003881EC, 0xE12FFF1E);
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
  Graphics &graphics = Graphics::GetInstance();
  menu::PluginMenu &menu = menu::PluginMenu::GetInstance();

  // Updates the logic based on user input.
  menu.Update();

  CheatCodeManager::GetInstance().Update();

  if (!menu.IsOpened()) return;

  // Renders the top screen.
  void *top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    Graphics::EnableScissor(0, 0, 400, 240);
    Graphics::BeginRender(top_buffer);
    menu.DrawTop();
    Graphics::DisableScissor();
  }

  // Renders the bottom screen.
  void *bottom_buffer = graphics.GetFramebuffer(Screen::kBottom);
  if (graphics.BindFramebuffer(bottom_buffer)) {
    Graphics::EnableScissor(0, 0, 320, 240);
    Graphics::BeginRender(bottom_buffer);
    menu.DrawBottom();
    Graphics::DisableScissor();
  }

  // Controller &ctrl = Controller::GetInstance();
  // if (ctrl.IsKeyPressed(Key::kR)) {
  //   Vec3 pos = {0, 0, 0};
  //   menu::LogMenu::GetInstance().Add(u"Teleport");
  //   ((void (*)(GameManager *, u16, Vec3 *, u8, bool, bool, bool, bool, bool,
  //              bool))ADDRESS_CHANGE_MAP)(&GameManager::GetInstance(),
  //                                        120,  // zone
  //                                        &pos,
  //                                        1,  // dir
  //                                        true,
  //                                        true,  // same background music
  //                                        true,  // fade
  //                                        true, true,
  //                                        true  // show place name
  //   );
  // }
}

// Performs the system initialization and prepares the plugin environment.
extern "C" void Initialize() {
  // Clear BSS section to ensure all uninitialized global variables start at
  // zero.
  extern u32 __bss_start__;
  extern u32 __bss_end__;
  for (u32 *data = &__bss_start__; data <= &__bss_end__; ++data) {
    *data = 0;
  }

  // Function pointer arrays used to call global constructors and static
  // initializers.
  extern void (*__preinit_array_start[])() __attribute__((weak));
  extern void (*__preinit_array_end[])() __attribute__((weak));
  extern void (*__init_array_start[])() __attribute__((weak));
  extern void (*__init_array_end[])() __attribute__((weak));

  // Execute pre-initialization arrays (rarely used, but handled by the CRT).
  const u32 preinit_count = __preinit_array_end - __preinit_array_start;
  for (u32 i = 0; i < preinit_count; ++i) {
    __preinit_array_start[i]();
  }

  // Execute initialization arrays: this calls constructors for global and
  // static C++ objects.
  const u32 init_count = __init_array_end - __init_array_start;
  for (u32 i = 0; i < init_count; ++i) {
    __init_array_start[i]();
  }

  File::MountSdmc();
  menu::PluginMenu::GetInstance().EnterSubMenu(MainMenu, nullptr);
  ApplyPatches();
  SetupCheatCodes();

  // Set a flag to ensure the initialization process is only executed once.
  WRITE(u32, ADDRESS_TARGET - 4, 1);
}

// Cleans up the plugin environment and executes termination arrays.
extern "C" void Finalize() {
  extern void (*__fini_array_start[])() __attribute__((weak));
  extern void (*__fini_array_end[])() __attribute__((weak));

  // Execute termination arrays: this calls destructors for global and static
  // C++ objects.
  const u32 fini_count = __fini_array_end - __fini_array_start;
  for (u32 i = fini_count; i > 0; i--) {
    __fini_array_start[i - 1]();
  }
}