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

#include "feature/feature_battle_config.h"
#include "feature/feature_camera.h"
#include "feature/feature_day_care.h"
#include "ui/page/page_top.h"
#include "feature/feature_engine.h"
#include "feature/feature_light.h"
#include "feature/feature_picture.h"
#include "feature/feature_pokemon_texture.h"
#include "feature/feature_text_box.h"
#include "feature/feature_device.h"
#include "feature/feature_map_tile.h"
#include "feature/feature_texture.h"
#include "feature/feature_overworld_model.h"
#include "feature/feature_app.h"
#include "feature/feature_battle.h"
#include "feature/feature_config.h"
#include "feature/feature_encounter.h"
#include "feature/feature_field_move.h"
#include "feature/feature_item.h"
#include "feature/feature_process.h"
#include "feature/feature_shop.h"
#include "ui/main_application.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "ui/application_manager.h"
#include "ui/root_application.h"

String String::s_tmp;
c16 String::s_buffer[128];

void Initialize() {
  File::MountSdmc();

  feature::DeviceHookContext::Initialize();
  feature::EngineHookContext::Initialize();
  feature::LightHookContext::Initialize();
  feature::TextBoxHookContext::Initialize();
  feature::PictureHookContext::Initialize();
  feature::PokemonTextureHookContext::Initialize();
  feature::BattleConfigHookContext::Initialize();
  feature::DayCareCheatCode::Initialize();
  feature::MapTileHookContext::Initialize();
  feature::CameraHookContext::Initialize();
  feature::OverworldModelCheatCode::Initialize();
  feature::AppHookContext::Initialize();
  feature::BattleHookContext::Initialize();
  feature::EncounterCheatCode::Initialize();
  feature::FieldMove::Initialize();
  feature::TextureHookContext::Initialize();
  feature::Config::Initialize();
  feature::Shop::Initialize();
  feature::Item::Initialize();

  auto& application_manager = ui::ApplicationManager::GetInstance();
  auto& root_app = ui::RootApplication::GetInstance();
  auto& main_app = ui::MainApplication::GetInstance();

#if USE_NUZLOCKE_MENU == 1
  main_app.SetPainter(ui::RetroAppPainter::GetInstance());
  main_app.Open(ui::LoadNuzlockePage);

  auto& light = feature::LightHookContext::GetInstance();
  light.use_outline = true;
  light.outline_scale = 0.0f;
#else
  main_app.SetPainter(ui::MainAppPainter::GetInstance());
  main_app.Open(ui::LoadTopPage);
#endif

  application_manager.Push(root_app);
}

void Entrypoint() {
  auto& graphics = Graphics::GetInstance();
  auto& controller = Controller::GetInstance();
  auto& application_manager = ui::ApplicationManager::GetInstance();
  auto& cheat_code_manager = CheatCodeManager::GetInstance();
  auto* application = application_manager.GetCurrentApplication();

  application->Update(controller);
  cheat_code_manager.Update();
  feature::ProcessHookContext::DoEachFrame();

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