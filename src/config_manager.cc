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

#include "feature/feature_battle.h"
#include "feature/feature_battle_config.h"
#include "feature/feature_engine.h"
#include "feature/feature_h3d_model.h"
#include "feature/feature_light.h"
#include "feature/feature_map_tile.h"
#include "feature/feature_picture.h"
#include "feature/feature_script.h"
#include "feature/feature_text_box.h"
#include "feature/feature_title_screen.h"
#include "system/file.h"
#include "ui/theme.h"

static const c16* kConfigFilename =
    u"sdmc:/luma/plugins/000400000011C500/sango.cfg";

static const u32 kConfigVersion = 3;

bool ConfigManager::Load() {
  auto& theme = ui::Theme::GetInstance();
  auto& battle = feature::Battle::GetInstance();
  auto& battle_config = feature::BattleConfig::GetInstance();
  auto& camera = feature::Camera::GetInstance();
  auto& engine = feature::Engine::GetInstance();
  auto& h3d = feature::H3dModel::GetInstance();
  auto& light = feature::Light::GetInstance();
  auto& map_tile = feature::MapTile::GetInstance();
  auto& overworld = feature::Overworld::GetInstance();
  auto& picture = feature::Picture::GetInstance();
  auto& script = feature::Script::GetInstance();
  auto& text_box = feature::TextBox::GetInstance();
  auto& title_screen = feature::TitleScreen::GetInstance();

#define FILE_READ(x) size = file.Read(&x, sizeof(x)); if(size < sizeof(x)) return false;

  // WARNING: There is some issues with File & game save feature
  u32 size = 0;
  u32 version = 0;
  File file(kConfigFilename, false);
  FILE_READ(version);
  if (version != kConfigVersion) {
    return false;
  }
  FILE_READ(theme);
  FILE_READ(battle);
  FILE_READ(battle_config);
  FILE_READ(camera);
  FILE_READ(engine);
  FILE_READ(h3d);
  FILE_READ(light);
  FILE_READ(map_tile);
  FILE_READ(overworld);
  FILE_READ(picture);
  FILE_READ(script);
  FILE_READ(text_box);
  FILE_READ(title_screen);

#undef FILE_READ

  return true;
}

void ConfigManager::Save(void*) {
  auto& theme = ui::Theme::GetInstance();
  auto& battle = feature::Battle::GetInstance();
  auto& battle_config = feature::BattleConfig::GetInstance();
  auto& camera = feature::Camera::GetInstance();
  auto& engine = feature::Engine::GetInstance();
  auto& h3d = feature::H3dModel::GetInstance();
  auto& light = feature::Light::GetInstance();
  auto& map_tile = feature::MapTile::GetInstance();
  auto& overworld = feature::Overworld::GetInstance();
  auto& picture = feature::Picture::GetInstance();
  auto& script = feature::Script::GetInstance();
  auto& text_box = feature::TextBox::GetInstance();
  auto& title_screen = feature::TitleScreen::GetInstance();

#define FILE_WRITE(x) file.Write(&x, sizeof(x))

  // WARNING: There is some issues with File & game save feature
  File file(kConfigFilename, true);
  FILE_WRITE(kConfigVersion);
  FILE_WRITE(theme);
  FILE_WRITE(battle);
  FILE_WRITE(battle_config);
  FILE_WRITE(camera);
  FILE_WRITE(engine);
  FILE_WRITE(h3d);
  FILE_WRITE(light);
  FILE_WRITE(map_tile);
  FILE_WRITE(overworld);
  FILE_WRITE(picture);
  FILE_WRITE(script);
  FILE_WRITE(text_box);
  FILE_WRITE(title_screen);

#undef FILE_WRITE
}