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

#include "config_manager.h"
#include "kaizo.h"
#include "feature/feature_battle_config.h"
#include "feature/feature_camera.h"
#include "ui/page/page_top.h"
#include "feature/feature_engine.h"
#include "feature/feature_light.h"
#include "feature/feature_picture.h"
#include "feature/feature_text_box.h"
#include "feature/feature_device.h"
#include "feature/feature_overworld_model.h"
#include "feature/feature_app.h"
#include "feature/feature_app_status.h"
#include "feature/feature_archive.h"
#include "feature/feature_battle.h"
#include "feature/feature_encounter.h"
#include "feature/feature_field_move.h"
#include "feature/feature_h3d_model.h"
#include "feature/feature_item.h"
#include "feature/feature_map_data_loader.h"
#include "feature/feature_map_tile.h"
#include "feature/feature_overworld.h"
#include "feature/feature_process_patch.h"
#include "feature/feature_script.h"
#include "ui/main_application.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "ui/application_manager.h"
#include "ui/root_application.h"

String String::s_tmp;
c16 String::s_buffer[128];

extern void UpdateOverworldWeather();
extern void InitOverworldWeather();
extern void Entrypoint();
extern void UpdateFollowingPokemon();

void Initialize() {
  File::MountSdmc();
  ConfigManager::Load();

  feature::DeviceState::Initialize();
  feature::Engine::Initialize();
  feature::Light::Initialize();
  feature::TextBox::Initialize();
  feature::Picture::Initialize();
  feature::MapTile::Initialize();
  feature::Camera::Initialize();
  feature::FieldMove::Initialize();
  feature::Item::Initialize();
  feature::Overworld::Initialize();
  feature::MapDataLoader::Initialize();
  feature::Encounter::Initialize();
  feature::ArchivePatch::Initiliaze();
  feature::Script::Initialize();
  feature::BattleConfig::Initialize();
  feature::H3dModel::Initialize();
  feature::Battle::Initialize();
  feature::OverworldModel::Initialize();
  feature::ProcessPatch::Initialize();
  feature::Keyboard::Initialize();
  feature::GameApp::Initialize();
  feature::AppStatus::Initialize();

  // feature::GameTextManager::Initialize();

#ifdef KAIZO
  kaizo::Initialize();
#endif

  auto& application_manager = ui::ApplicationManager::GetInstance();
  auto& root_app = ui::RootApplication::GetInstance();
  auto& main_app = ui::MainApplication::GetInstance();

#ifdef  KAIZO
  main_app.SetPainter(ui::KaizoAppPainter::GetInstance());
  main_app.Open(ui::LoadKaizoPage);
#else
  main_app.SetPainter(ui::MainAppPainter::GetInstance());
  main_app.Open(ui::LoadTopPage);
#endif

  application_manager.Push(root_app);

  HookManager::Initialize(HookID::kEntrypoint, ADDRESS_ENTRYPOINT,
                          (uptr)Entrypoint);
}

void Entrypoint() {
  auto& graphics = Graphics::GetInstance();
  auto& controller = Controller::GetInstance();
  auto& application_manager = ui::ApplicationManager::GetInstance();
  auto& cheat_code_manager = CheatCodeManager::GetInstance();
  auto* application = application_manager.GetCurrentApplication();

  application->Update(controller);
  cheat_code_manager.Update();
#ifdef KAIZO
  kaizo::UpdateOverworldWeather();
#endif

  // UpdateFollowingPokemon();

  void* top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    Graphics::EnableScissor(0, 0, 400, 240);
    Graphics::BeginRender(top_buffer);
    application->DrawTop(graphics);
    feature::Keyboard::DrawTop();
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

bool MemoryManager::Unprotect(u32 address, u32 size) {
  return ToggleProtection(address, true);
}

bool MemoryManager::Protect(u32 address, u32 size) {
  return ToggleProtection(address, false);
}

bool MemoryManager::ToggleProtection(u32 address, bool on) {
  u32 pID;
  if (R_FAILED(svcGetProcessId(&pID, CUR_PROCESS_HANDLE))) {
    return false;
  }

  Handle processHandle;
  if (R_FAILED(svcOpenProcess(&processHandle, pID))) {
    return false;
  }

  MemInfo mInfo;
  PageInfo pInfo;
  if (R_FAILED(svcQueryMemory(&mInfo, &pInfo, address))) {
    svcCloseHandle(processHandle);
    return false;
  }

  MemPerm perm = on
                   ? MemPerm(MEMPERM_READ | MEMPERM_EXECUTE | MEMPERM_WRITE)
                   : MemPerm(MEMPERM_READ | MEMPERM_EXECUTE);

  Result res = svcControlProcessMemory(processHandle, mInfo.base_addr, 0,
                                       mInfo.size, MemOp(MEMOP_PROT), perm);

  // ui::LogApplication::Print(u"%s %X, %X, %X",
  //                           on ? "Unprotect" : "Protect",
  //                           mInfo.base_addr,
  //                           mInfo.size, processHandle);

  svcCloseHandle(processHandle);
  return R_SUCCEEDED(res);
}