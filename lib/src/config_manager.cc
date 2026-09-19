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

#include "battle/patch/battle.h"
#include "battle/patch/setup.h"
#include "core/patch/game_speed.h"
#include "core/patch/script_loader.h"
#include "overworld/patch/camera.h"
#include "overworld/patch/field.h"
#include "overworld/patch/map_tile.h"
#include "overworld/patch/weather_override.h"
#include "pokemon/patch/shiny.h"
#include "renderer/patch/lighting.h"
#include "renderer/patch/model_filter.h"
#include "renderer/patch/picture_filter.h"
#include "renderer/patch/text_box_filter.h"
#include "system/native/file.h"
#include "ui/theme.h"
#include "ui/patch/title_screen.h"

static const c16* kConfigFilename =
    u"sdmc:/luma/plugins/000400000011C500/sango.cfg";

// Bump whenever one of the *Settings structs below changes layout.
static constexpr u32 kConfigVersion = 13;

// Only the *Settings base of each feature goes through the file: the rest of
// the singleton (callbacks, counters, runtime state) belongs to the product
// and must survive a Load().
#define FOR_EACH_SETTINGS(X)                                                \
  X(ui::Theme, ui::Theme)                                                   \
  X(battle::BattleSettings, battle::Battle)                                 \
  X(battle::SetupSettings, battle::Setup)                                   \
  X(overworld::CameraSettings, overworld::Camera)                           \
  X(core::GameSpeedSettings, core::GameSpeed)                           \
  X(renderer::ModelFilterSettings, renderer::ModelFilter)                   \
  X(renderer::LightingSettings, renderer::Lighting)                         \
  X(overworld::MapTileSettings, overworld::MapTile)                         \
  X(overworld::FieldSettings, overworld::Field)                             \
  X(renderer::PictureFilterSettings, renderer::PictureFilter)               \
  X(core::ScriptLoaderSettings, core::ScriptLoader)                           \
  X(renderer::TextBoxFilterSettings, renderer::TextBoxFilter)               \
  X(ui::TitleScreenSettings, ui::TitleScreen)                               \
  X(pokemon::ShinySettings, pokemon::Shiny)                                 \
  X(overworld::WeatherOverrideSettings, overworld::WeatherOverride)

bool ConfigManager::Load() {
  // WARNING: There is some issues with sys::File & game save feature
  sys::File file(kConfigFilename, false);
  u32 version = 0;
  if (file.Read(&version, sizeof(version)) < (s32)sizeof(version)) {
    return false;
  }
  if (version != kConfigVersion) {
    return false;
  }

#define READ_SETTINGS(Settings, Feature)                                    \
  {                                                                         \
    Settings& settings = Feature::GetInstance();                            \
    if (file.Read(&settings, sizeof(settings)) < (s32)sizeof(settings))     \
      return false;                                                         \
  }
  FOR_EACH_SETTINGS(READ_SETTINGS)
#undef READ_SETTINGS

  return true;
}

void ConfigManager::Save(void*) {
  // WARNING: There is some issues with sys::File & game save feature
  sys::File file(kConfigFilename, true);
  file.Write(&kConfigVersion, sizeof(kConfigVersion));

#define WRITE_SETTINGS(Settings, Feature)                                   \
  {                                                                         \
    const Settings& settings = Feature::GetInstance();                      \
    file.Write(&settings, sizeof(settings));                                \
  }
  FOR_EACH_SETTINGS(WRITE_SETTINGS)
#undef WRITE_SETTINGS
}

#undef FOR_EACH_SETTINGS
