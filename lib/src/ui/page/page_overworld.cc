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

#include <cmath>

#include "core/native/data_manager.h"
#include "core/native/process_manager.h"
#include "overworld/native/encounter_state.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/prop_model_manager.h"
#include "overworld/native/weather_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/field.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/map_data_loader.h"
#include "overworld/patch/map_graft.h"
#include "overworld/patch/map_tile.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/weather_override.h"
#include "renderer/native/h3d_shader_model.h"
#include "system/native/sound.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"
#include "ui/tile_editor_application.h"

namespace ui {
#include "overworld/data/tile.inc"

#ifdef GAME_XY
static u8 forced_weather = 0;

static void ApplyForcedWeather(void*) {
  overworld::WeatherManager::GetInstance().ForceWeather(
      static_cast<overworld::Weather>(forced_weather),
      core::DataManager::GetInstance().GetSeason());
}

static void ReleaseForcedWeather(void*) {
  overworld::WeatherManager::GetInstance().ReleaseWeather();
}
#endif

void LoadWeatherPage(MainApplication& app, void* args) {
#ifdef GAME_XY
  static const c8* WEATHERS[] = {
      "Sunny", "Sunny 2", "Sunny 3", "Sunny 4",
      "Sunny 5", "Sunny 6", "Sunny 7",
      "Sunny 8", "Sunny 9", "Sunny 10",
      "Sunny 11", "Sunny 12", "Sunny 13",
      "Cloudy", "Light Rain", "Rain", "Heavy Rain", "Light Snow", "Snow",
      "Heavy Snow", "Snowstorm", "Diamond Dust", "Sunny Wind", "None"
  };
#else
  static const c8* WEATHERS[] = {
      "Sunny", "Rainy", "Thunderstorm", "Misty", "Ash", "Sandstorm",
      "Cloudy", "Stormy", "Dry", "None"
  };
#endif
  static const c8* RAIN_MODES[] = {"Normal", "Toxic", "Radioactive"};

  auto& ctx = overworld::WeatherOverride::GetInstance();

  if (core::ProcessManager::IsOverworldActive()) {
    auto& manager = overworld::WeatherManager::GetInstance();
#ifdef GAME_XY
    // The requested weather is rewritten by the game every frame on X/Y:
    // show the current one and go through the force request instead.
    app.Add("Current Weather", manager.GetCurrentWeather())
       .WithArray(WEATHERS, SIZE(WEATHERS))
       .WithBounds(0, SIZE(WEATHERS) - 1)
       .Add("Forced Weather", forced_weather)
       .WithArray(WEATHERS, SIZE(WEATHERS))
       .WithBounds(0, SIZE(WEATHERS) - 1)
       .Add("Apply Forced Weather", ApplyForcedWeather)
       .Add("Release Forced Weather", ReleaseForcedWeather)
       .AddSeparator();
#else
    app.Add("Current Weather", manager.GetRequestedWeather())
       .WithArray(WEATHERS, SIZE(WEATHERS))
       .WithBounds(0, SIZE(WEATHERS) - 1)
       .AddSeparator();
#endif
  }

  app.Add("Keep Weather Indoors", ctx.keep_weather_indoors)
     .Add("Ignore Zone Weather", ctx.ignore_zone_weather)
     .Add("Rain Mode", ctx.mode)
     .WithArray(RAIN_MODES, SIZE(RAIN_MODES))
     .WithBounds(0, SIZE(RAIN_MODES) - 1);
}

void LoadOverworldMapTilePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& ctx = overworld::MapTile::GetInstance();

  app.Add("Override Tiles", ctx.is_enabled)
     .AddSeparator()
     .Add("Is Impassable", ctx.is_impassable)
     .Add("Is Water", ctx.is_water)
     .Add("Permits Encounters", ctx.permits_encounters)
     .Add("Allows Diagonal Movement", ctx.allows_diagonal_movement)
     .Add("Has Shadow", ctx.has_shadow)
     .Add("Blocks Trainer Movement", ctx.blocks_trainer_movement)
     .Add("Can Place Decorations", ctx.can_place_decorations)
     .Add("Can Move Decoration Cursor", ctx.can_move_decoration_cursor)
     .AddSeparator()
     .Add("Reflection 0", ctx.has_reflection_0)
     .Add("Reflection 1", ctx.has_reflection_1)
     .Add("Reflection 2", ctx.has_reflection_2)
     .Add("Reflection 3", ctx.has_reflection_3)
     .AddSeparator()
     .Add("Footstep Sound Type", ctx.foot_step_id)
     .WithArray(FOOT_STEPS, SIZE(FOOT_STEPS))
     .Add("Battle Background Type", ctx.battle_background_id)
     .Add("Ground Type", ctx.ground_id)
     .WithArray(GROUNDS, SIZE(GROUNDS));
}

void LoadOverworldEncounterPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& state = overworld::EncounterState::GetInstance();

  app.Add("Contact Encounters",
          overworld::MapDataLoader::GetInstance().is_contact_enabled)
     .AddSeparator()
     .Add("Walk Count", state.walk_count)
     .Add("Encounter Rate", state.encounter_rate)
     .Add("Fishing Chain", state.fishing_chain_count);
}

void LoadPropModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;
  auto& manager = overworld::PropModelManager::GetInstance();
  if (manager.count == 0) {
    app.Add("There is no prop here...");
    return;
  }

  static const char* PROP_SOUND_EFFECTS[] = {
      "None",
      "Normal Door",
      "Push Door",
      "Double Door",
      "Automatic Door",
      "Pokemon Center",
      "Big Double Door",
      "Temple Door",
      "Metal Door",
      "Elite Four Door"
  };

  static u32 choice = 0;

  app.Add("Prop Index", choice)
     .WithBounds(0, manager.count - 1)
     .WithRefresh();

  auto& prop = manager.prop_models[choice];
  if (!prop.is_initialized) return;

  auto* shader = manager.prop_models[choice].shader;

  app.AddSeparator()
     .Add("Is Visible", shader->_0[0xE4])
     .Add("Sound Effect", prop.sound_effect)
     .WithArray(PROP_SOUND_EFFECTS, SIZE(PROP_SOUND_EFFECTS))
     .AddSeparator()
     .Add("Scale X", shader->scale.x)
     .WithFactor(0.2f)
     .Add("Scale Y", shader->scale.y)
     .WithFactor(0.2f)
     .Add("Scale Z", shader->scale.z)
     .WithFactor(0.2f)
     .AddSeparator()
     .Add("Rotation X", shader->rotation.x)
     .WithFactor(M_PI / 12.0f)
     .Add("Rotation Y", shader->rotation.y)
     .WithFactor(M_PI / 12.0f)
     .Add("Rotation Z", shader->rotation.z)
     .WithFactor(M_PI / 12.0f)
     .AddSeparator()
     .Add("Position X", shader->position.x)
     .Add("Position Y", shader->position.y)
     .Add("Position Z", shader->position.z);
}

void LoadDayCarePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& day_care = overworld::DayCare::GetInstance();

  app.Add("Instant Egg Hatch", day_care.instant_egg_hatch)
     .WithCallback(overworld::DayCare::ApplyEggHatch)
     .Add("Instant Max Exp", day_care.instant_max_exp)
     .WithCallback(overworld::DayCare::ApplyMaxExp);
}

void LoadWorldLayoutPage(MainApplication& app, void* args) {
  using overworld::WorldLayout;
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const c8* SIDES[] = {"Up", "Down", "Left", "Right"};
  static const c8* GRAFT_NAMES[overworld::MapGraft::kMaxRequests] = {
      "Graft 0", "Graft 1", "Graft 2", "Graft 3",
      "Graft 4", "Graft 5", "Graft 6", "Graft 7"
  };
  static const c8* LINK_NAMES[overworld::MapGraft::kMaxLinks] = {
      "Link 0", "Link 1", "Link 2", "Link 3",
      "Link 4", "Link 5", "Link 6", "Link 7"
  };

  auto& layout = WorldLayout::GetInstance();
  auto& graft = overworld::MapGraft::GetInstance();

  app.Add("Layout Id", layout.id)
     .Add("Graft Enabled", graft.is_enabled)
     .Add("Graft Logging", graft.is_logging_enabled)
     .Add("Character Logging",
          overworld::MapCharacter::GetInstance().is_logging_enabled)
     .Add("Apply Grafts (Reload Map)", RefreshMap);

  app.AddSeparator()
     .Add("Tile Editor", [](void*) { TileEditorApplication::Open(); })
     .Add("Tile Edits Enabled", overworld::TileEditor::GetInstance().is_enabled)
     .Add("Save Tile Edits", [](void*) { overworld::TileEditor::Save(); })
     .Add("Clear Tile Edits",
          [](void*) { overworld::TileEditor::ClearEdits(); });

  for (u32 i = 0; i < overworld::MapGraft::GetCount(); i++) {
    auto& request = overworld::MapGraft::GetRequest(i);
    auto& result = overworld::MapGraft::GetResult(i);
    app.AddSeparator()
       .Add(GRAFT_NAMES[i], result.is_applied)
       .Add("Anchor Map", request.anchor)
       .Add("Side", request.side)
       .WithArray(SIDES, SIZE(SIDES))
       .WithBounds(0, SIZE(SIDES) - 1)
       .Add("Grafted Map", request.map)
       .Add("Shift (blocks)", request.shift)
       .WithMin(-31)
       .WithMax(31)
       .Add("Offset X (blocks)", result.offset_x)
       .Add("Offset Z (blocks)", result.offset_z);
  }

  for (u32 i = 0; i < overworld::MapGraft::GetLinkCount(); i++) {
    auto& link = overworld::MapGraft::GetLink(i);
    app.AddSeparator()
       .Add(LINK_NAMES[i])
       .Add("Leaving Map", link.from)
       .Add("Through Side", link.side)
       .WithArray(SIDES, SIZE(SIDES))
       .WithBounds(0, SIZE(SIDES) - 1)
       .Add("Lands On Map", link.to)
       .Add("Tile X", link.tile_x)
       .WithBounds(0, WorldLayout::kMaxWidth * WorldLayout::kTilesPerBlockSide)
       .Add("Tile Z", link.tile_z)
       .WithBounds(
           0, WorldLayout::kMaxHeight * WorldLayout::kTilesPerBlockSide);
  }
}

static void PlayFieldMusic(void*) {
  sys::Sound::PlayBackgroundMusic(
      overworld::Field::GetInstance().background_music);
}

void LoadOverworldPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& map_manager = overworld::MapManager::GetInstance();
  auto& field = overworld::Field::GetInstance();

  app.Add("Map Id", map_manager.GetMapId())
     .Add("Reload Map", RefreshMap)
     .AddSeparator()
     .Add("Weather", LoadWeatherPage)
     .Add("World Layout", LoadWorldLayoutPage)
     .Add("Map Tile", LoadOverworldMapTilePage)
     .Add("Props", LoadPropModelPage)
     .Add("Encounter", LoadOverworldEncounterPage)
     .Add("Day Care", LoadDayCarePage)
     .AddSeparator()
     .Add("Freeze Background Music", field.freeze_background_music)
     .Add("Background Music", field.background_music)
     .WithBounds(0, static_cast<u32>(BackgroundMusicId::kCount) - 1)
     .WithCallback(PlayFieldMusic);
}
} // namespace ui