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

#include "plugin.h"

#include "config_manager.h"
#include "feature/battle/feature_battle.h"
#include "feature/battle/feature_battle_config.h"
#include "feature/battle/feature_game_extension.h"
#include "feature/battle/feature_type_helper.h"
#include "feature/core/feature_app.h"
#include "feature/core/feature_archive.h"
#include "feature/core/feature_device.h"
#include "feature/core/feature_engine.h"
#include "feature/core/feature_event_patch.h"
#include "feature/core/feature_native_script.h"
#include "feature/core/feature_process_patch.h"
#include "feature/core/feature_script.h"
#include "feature/overworld/feature_camera.h"
#include "feature/overworld/feature_encounter.h"
#include "feature/overworld/feature_field_move.h"
#include "feature/overworld/feature_map_character.h"
#include "feature/overworld/feature_map_data_loader.h"
#include "feature/overworld/feature_map_graft.h"
#include "feature/overworld/feature_map_tile.h"
#include "feature/overworld/feature_overworld.h"
#include "feature/overworld/feature_overworld_model.h"
#include "feature/overworld/feature_static_encounter.h"
#include "feature/overworld/feature_tile_editor.h"
#include "feature/overworld/feature_trade.h"
#include "feature/pokemon/feature_evolution.h"
#include "feature/pokemon/feature_item.h"
#include "feature/pokemon/feature_mega_evolution.h"
#include "feature/pokemon/feature_shiny.h"
#include "feature/pokemon/feature_shop.h"
#include "feature/rendering/feature_h3d_model.h"
#include "feature/rendering/feature_light.h"
#include "feature/rendering/feature_picture.h"
#include "feature/rendering/feature_text_box.h"
#include "feature/ui/feature_app_status.h"
#include "feature/ui/feature_keyboard.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "ui/application_manager.h"
#include "ui/main_application.h"
#include "ui/root_application.h"

String String::s_tmp;
c16 String::s_buffer[128];

namespace plugin {
void InitializeEngine() {
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
  feature::NativeScript::Initialize();
  feature::BattleConfig::Initialize();
  feature::H3dModel::Initialize();
  feature::Battle::Initialize();
  feature::OverworldModel::Initialize();
  feature::ProcessPatch::Initialize();
  feature::EventPatch::Initialize();
  feature::Keyboard::Initialize();
  feature::GameApp::Initialize();
  feature::AppStatus::Initialize();
  feature::Shiny::Initialize();
  feature::GameExtension::Initialize();
  feature::MegaEvolution::Initialize();
  feature::Evolution::Initialize();
  feature::GiftPokemon::Initialize();
  feature::StaticEncounter::Initialize();
  feature::Trade::Initialize();
  feature::MapCharacter::Initialize();
  feature::MapGraft::Initialize();
  feature::TileEditor::Initialize();
  feature::Shop::Initialize();
}

void LoadConfiguration() {
  ConfigManager::Load();
}

void OpenMenu(ui::Painter& painter, PageLoader root_page) {
  auto& main_app = ui::MainApplication::GetInstance();
  main_app.SetPainter(painter);
  main_app.Open(root_page);
  ui::ApplicationManager::GetInstance().Push(ui::RootApplication::GetInstance());
}

void Start(void (*every_frame)()) {
  HookManager::Initialize(HookID::kEntrypoint, ADDRESS_ENTRYPOINT,
                          (uptr)every_frame);
}

void UpdateFrame() {
  auto& controller = Controller::GetInstance();
  auto* application = ui::ApplicationManager::GetInstance().GetCurrentApplication();

  application->Update(controller);
  CheatCodeManager::GetInstance().Update();

  feature::MapCharacter::Update();
  feature::MapGraft::Update();
  feature::TileEditor::Update();
}

void DrawFrame() {
  auto& graphics = Graphics::GetInstance();
  auto* application = ui::ApplicationManager::GetInstance().GetCurrentApplication();

  void* top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    Graphics::EnableScissor(0, 0, 400, 240);
    Graphics::BeginRender(top_buffer);
    application->DrawTop(graphics);
    feature::Keyboard::DrawTop();
    feature::TypeHelper::DrawTop();
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
} // namespace plugin
