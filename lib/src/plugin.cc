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

#include "battle/patch/battle.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/setup.h"
#include "battle/patch/type_helper.h"
#include "config_manager.h"
#include "core/patch/app_launcher.h"
#include "core/patch/archive.h"
#include "core/patch/device_patch.h"
#include "core/patch/event_patch.h"
#include "core/patch/game_speed.h"
#include "core/patch/process_patch.h"
#include "core/patch/script_loader.h"
#include "overworld/patch/camera.h"
#include "overworld/patch/field.h"
#include "overworld/patch/field_move.h"
#include "overworld/patch/gift_pokemon.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/map_data_loader.h"
#include "overworld/patch/map_graft.h"
#include "overworld/patch/map_tile.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/patch/run_animation.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/trade.h"
#include "overworld/patch/weather_override.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/patch/custom_shop.h"
#include "pokemon/patch/evolution.h"
#include "pokemon/patch/item_customizer.h"
#include "pokemon/patch/mega_evolution.h"
#include "pokemon/patch/model_replacement.h"
#include "pokemon/patch/shiny.h"
#include "renderer/patch/lighting.h"
#include "renderer/patch/model_filter.h"
#include "renderer/patch/picture_filter.h"
#include "renderer/patch/text_box_filter.h"
#include "script/patch/native_script.h"
#include "system/native/controller.h"
#include "system/native/file.h"
#include "system/native/graphics.h"
#include "ui/application_manager.h"
#include "ui/main_application.h"
#include "ui/patch/app_status.h"
#include "ui/patch/keyboard_patch.h"
#include "ui/patch/new_game.h"
#include "ui/patch/title_screen.h"
#include "ui/root_application.h"

sys::String sys::String::s_tmp;
c16 sys::String::s_buffer[128];

namespace plugin {
void InitializeEngine() {
  sys::File::MountSdmc();

  core::DevicePatch::Initialize();
  core::GameSpeed::Initialize();
  renderer::Lighting::Initialize();
  renderer::TextBoxFilter::Initialize();
  renderer::PictureFilter::Initialize();
  overworld::MapTile::Initialize();
  overworld::Camera::Initialize();
  overworld::FieldMove::Initialize();
  pokemon::ItemCustomizer::Initialize();
  overworld::Field::Initialize();
  overworld::MapDataLoader::Initialize();
  overworld::WildEncounter::Initialize();
  core::Archive::Initialize();
  core::ScriptLoader::Initialize();
  script::NativeScript::Initialize();
  battle::Setup::Initialize();
  renderer::ModelFilter::Initialize();
  battle::Battle::Initialize();
  overworld::PlayerCheats::Initialize();
  core::ProcessPatch::Initialize();
  core::EventPatch::Initialize();
  ui::KeyboardPatch::Initialize();
  core::AppLauncher::Initialize();
  ui::AppStatus::Initialize();
  pokemon::Shiny::Initialize();
  battle::GameExtension::Initialize();
  pokemon::MegaEvolution::Initialize();
  pokemon::Evolution::Initialize();
  overworld::GiftPokemon::Initialize();
  overworld::StaticRandomizer::Initialize();
  overworld::Trade::Initialize();
  overworld::MapCharacter::Initialize();
  overworld::MapGraft::Initialize();
  overworld::TileEditor::Initialize();
  pokemon::CustomShop::Initialize();
  overworld::WeatherOverride::Initialize();
  ui::NewGame::Initialize();
  overworld::RunAnimation::Initialize();
  pokemon::ModelReplacement::Initialize();
  ui::TitleScreen::Initialize();
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
  core::HookManager::Initialize(HookId::kEntrypoint, sys::address::kEntrypoint,
                          (uptr)every_frame);
}

void UpdateFrame() {
  auto& controller = sys::Controller::GetInstance();
  auto* application = ui::ApplicationManager::GetInstance().GetCurrentApplication();

  application->Update(controller);
  core::CheatCodeManager::GetInstance().Update();

  overworld::MapCharacter::Update();
  overworld::MapGraft::Update();
  overworld::TileEditor::Update();
}

void DrawFrame() {
  auto& graphics = sys::Graphics::GetInstance();
  auto* application = ui::ApplicationManager::GetInstance().GetCurrentApplication();

  void* top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    sys::Graphics::EnableScissor(0, 0, 400, 240);
    sys::Graphics::BeginRender(top_buffer);
    application->DrawTop(graphics);
    ui::KeyboardPatch::DrawTop();
    battle::TypeHelper::DrawTop();
    sys::Graphics::DisableScissor();
  }

  void* bottom_buffer = graphics.GetFramebuffer(Screen::kBottom);
  if (graphics.BindFramebuffer(bottom_buffer)) {
    sys::Graphics::EnableScissor(0, 0, 320, 240);
    sys::Graphics::BeginRender(bottom_buffer);
    application->DrawBottom(graphics);
    sys::Graphics::DisableScissor();
  }
}
} // namespace plugin
