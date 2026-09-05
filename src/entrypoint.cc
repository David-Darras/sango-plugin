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
#include "kaizo/kaizo.h"
#include "feature/battle/feature_game_extension.h"
#include "feature/battle/feature_battle_config.h"
#include "feature/overworld/feature_camera.h"
#include "ui/page/page_top.h"
#include "feature/core/feature_engine.h"
#include "feature/rendering/feature_light.h"
#include "feature/rendering/feature_picture.h"
#include "feature/rendering/feature_text_box.h"
#include "feature/core/feature_device.h"
#include "feature/overworld/feature_overworld_model.h"
#include "feature/core/feature_app.h"
#include "feature/ui/feature_app_status.h"
#include "feature/core/feature_archive.h"
#include "feature/battle/feature_battle.h"
#include "feature/overworld/feature_encounter.h"
#include "feature/overworld/feature_field_move.h"
#include "feature/rendering/feature_h3d_model.h"
#include "feature/pokemon/feature_item.h"
#include "feature/overworld/feature_map_data_loader.h"
#include "feature/overworld/feature_map_tile.h"
#include "feature/overworld/feature_overworld.h"
#include "feature/core/feature_process_patch.h"
#include "feature/core/feature_script.h"
#include "feature/pokemon/feature_evolution.h"
#include "feature/pokemon/feature_mega_evolution.h"
#include "feature/pokemon/feature_shiny.h"
#include "game/savedata/settings.h"
#include "ui/main_application.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "ui/application_manager.h"
#include "ui/root_application.h"

namespace kaizo {
extern void InitializeModelHook();
}

String String::s_tmp;
c16 String::s_buffer[128];

s32 mega_step = 0;

extern void UpdateOverworldWeather();
extern void InitOverworldWeather();
extern void Entrypoint();
extern void UpdateFollowingPokemon();

void Initialize() {
  File::MountSdmc();

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
  feature::Shiny::Initialize();
  feature::GameExtension::Initialize();
  feature::MegaEvolution::Initialize();
  feature::Evolution::Initialize();

#ifdef KAIZO
  kaizo::Initialize();
#else
  kaizo::InitializeModelHook();
#endif
  ConfigManager::Load();

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

  UpdateFollowingPokemon();

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