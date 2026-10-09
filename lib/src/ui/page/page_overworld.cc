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

/**
 * @file page_overworld.cc
 * @brief The menu pages of the Overworld family.
 */

#include <cmath>

#include "core/native/data_manager.h"
#include "core/native/process_manager.h"
#include "overworld/native/encounter_state.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/prop_model_manager.h"
#include "overworld/native/weather_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/overworld.h"
#include "overworld/patch/tall_grass.h"
#include "overworld/patch/healer_follower.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/map_data_loader.h"
#include "overworld/patch/map_graft.h"
#include "overworld/patch/map_tile.h"
#include "overworld/constant/decoration.h"
#include "overworld/patch/placed_decorations.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/weather_override.h"
#include "overworld/patch/world_overlay.h"
#include "pokemon/patch/model_loader.h"
#include "core/patch/time_override.h"
#include "overworld/native/hidden_item.h"
#include "renderer/native/h3d_shader_model.h"
#include "system/native/sound.h"
#include "ui/log_application.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"
#include "ui/tile_editor_application.h"

namespace ui {
#include "overworld/data/tile.inc"
#include "overworld/data/decoration_names.inc"

#ifdef GAME_XY
static u8 forced_weather = 0;

static void ApplyForcedWeather(void*) {
  overworld::WeatherManager::GetInstance().ForceWeather(
      static_cast<overworld::Weather>(forced_weather),
      core::DataManager::GetInstance().GetSeason());
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
    forced_weather = static_cast<u8>(manager.GetCurrentWeather());
    app.Add("Weather", forced_weather)
       .WithArray(WEATHERS, SIZE(WEATHERS))
       .WithBounds(0, SIZE(WEATHERS) - 1)
       .WithCallback(ApplyForcedWeather)
       .WithDescription("Select a weather, then press A to show it.");
#else
    app.Add("Current Weather", manager.GetRequestedWeather())
       .WithArray(WEATHERS, SIZE(WEATHERS))
       .WithBounds(0, SIZE(WEATHERS) - 1);
#endif
  }

  app.AddSection("Options")
     .Add("Keep Weather Indoors", ctx.keep_weather_indoors)
     .WithDescription("The weather continues in the buildings.")
     .Add("Ignore Zone Weather", ctx.ignore_zone_weather)
     .WithDescription("The maps do not change the weather.")
     .Add("Rain Mode", ctx.mode)
     .WithDescription("The color of the rain.")
     .WithArray(RAIN_MODES, SIZE(RAIN_MODES))
     .WithBounds(0, SIZE(RAIN_MODES) - 1);
}

static void SetTimeOfDay(u8 hour) {
  auto& ctx = core::TimeOverride::GetInstance();
  ctx.is_enabled = true;
  ctx.hour = hour;
  ctx.minute = 0;
}

void LoadTimeOfDayPage(MainApplication& app, void* args) {
  auto& ctx = core::TimeOverride::GetInstance();

  app.AddSection("Time")
     .Add("Override Time", ctx.is_enabled)
     .WithDescription("On: the game uses this time, not the time of the "
                      "console.")
     .Add("Hour", ctx.hour)
     .WithBounds(0, 23)
     .Add("Minute", ctx.minute)
     .WithBounds(0, 59)
     .AddSection("Quick Times")
     .Add("Morning (7:00)", [](void*) { SetTimeOfDay(7); })
     .Add("Day (12:00)", [](void*) { SetTimeOfDay(12); })
     .Add("Evening (18:00)", [](void*) { SetTimeOfDay(18); })
     .Add("Night (22:00)", [](void*) { SetTimeOfDay(22); })
     .Add("Midnight (0:00)", [](void*) { SetTimeOfDay(0); });
}

void LoadOverworldMapTilePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& ctx = overworld::MapTile::GetInstance();

  app.Add("Override Tiles", ctx.is_enabled)
     .WithDescription("On: all the tiles of the map use the attributes of "
                      "this page.")
     .AddSection("Movement")
     .Add("Is Impassable", ctx.is_impassable)
     .Add("Is Water", ctx.is_water)
     .Add("Permits Encounters", ctx.permits_encounters)
     .Add("Allows Diagonal Movement", ctx.allows_diagonal_movement)
     .Add("Blocks Trainer Movement", ctx.blocks_trainer_movement)
     .Add("Can Place Decorations", ctx.can_place_decorations)
     .Add("Can Move Decoration Cursor", ctx.can_move_decoration_cursor)
     .AddSection("Look")
     .Add("Has Shadow", ctx.has_shadow)
     .Add("Reflection 0", ctx.has_reflection_0)
     .Add("Reflection 1", ctx.has_reflection_1)
     .Add("Reflection 2", ctx.has_reflection_2)
     .Add("Reflection 3", ctx.has_reflection_3)
     .AddSection("Types")
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
     .AddSection("Current State")
     .Add("Walk Count", state.walk_count)
     .Add("Encounter Rate", state.encounter_rate)
     .Add("Fishing Chain", state.fishing_chain_count);
}

void LoadPropModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;
  auto& manager = overworld::PropModelManager::GetInstance();
  if (manager.count == 0) {
    app.Add("There is no prop on this map.");
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

  if (choice >= manager.count) choice = 0;
  app.Add("Prop", choice)
     .WithBounds(0, manager.count - 1)
     .WithRefresh();

  auto& prop = manager.prop_models[choice];
  if (!prop.is_initialized) return;

  auto* shader = manager.prop_models[choice].shader;

  app.Add("Is Visible", shader->_0[0xE4])
     .WithBounds(0, 1)
     .Add("Sound Effect", prop.sound_effect)
     .WithArray(PROP_SOUND_EFFECTS, SIZE(PROP_SOUND_EFFECTS))
     .AddSection("Scale")
     .Add("Scale X", shader->scale.x)
     .WithFactor(0.2f)
     .Add("Scale Y", shader->scale.y)
     .WithFactor(0.2f)
     .Add("Scale Z", shader->scale.z)
     .WithFactor(0.2f)
     .AddSection("Rotation")
     .Add("Rotation X", shader->rotation.x)
     .WithFactor(M_PI / 12.0f)
     .Add("Rotation Y", shader->rotation.y)
     .WithFactor(M_PI / 12.0f)
     .Add("Rotation Z", shader->rotation.z)
     .WithFactor(M_PI / 12.0f)
     .AddSection("Position")
     .Add("Position X", shader->position.x)
     .Add("Position Y", shader->position.y)
     .Add("Position Z", shader->position.z);
}

void LoadDayCarePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& day_care = overworld::DayCare::GetInstance();

  app.Add("Instant Egg Hatch", day_care.instant_egg_hatch)
     .WithCallback(overworld::DayCare::ApplyEggHatch)
     .WithDescription("Press A to switch it. The Eggs hatch at once.")
     .Add("Instant Max Exp", day_care.instant_max_exp)
     .WithCallback(overworld::DayCare::ApplyMaxExp)
     .WithDescription("Press A to switch it. The Pokemon of the Day Care get "
                      "the maximum experience.");
}

void LoadWorldLayoutPage(MainApplication& app, void* args) {
  using overworld::WorldLayout;
  if (app.CheckProcess(overworld::address::kVtable)) return;

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

  app.AddSection("Layout")
     .Add("Layout ID", layout.id)
     .Add("Graft Enabled", graft.is_enabled)
     .Add("Graft Logging", graft.is_logging_enabled)
     .Add("Character Logging",
          overworld::MapCharacter::GetInstance().is_logging_enabled)
     .Add("Apply Grafts (Reload Map)", RefreshMap);

  app.AddSection("Tile Editor")
     .Add("Tile Editor", [](void*) { TileEditorApplication::Open(); })
     .Add("Tile Edits Enabled", overworld::TileEditor::GetInstance().is_enabled)
     .Add("Save Tile Edits", [](void*) { overworld::TileEditor::Save(); })
     .Add("Clear Tile Edits",
          [](void*) { overworld::TileEditor::ClearEdits(); });

  for (u32 i = 0; i < overworld::MapGraft::GetCount(); i++) {
    auto& request = overworld::MapGraft::GetRequest(i);
    auto& result = overworld::MapGraft::GetResult(i);
    app.AddSection(GRAFT_NAMES[i])
       .Add("Is Applied", result.is_applied)
       .WithReadOnly()
       .Add("Anchor Map", request.anchor)
       .Add("Side", request.side)
       .WithArray(kSideNames, SIZE(kSideNames))
       .Add("Grafted Map", request.map)
       .Add("Shift (blocks)", request.shift)
       .WithMin(-31)
       .WithMax(31)
       .Add("Offset X (blocks)", result.offset_x)
       .Add("Offset Z (blocks)", result.offset_z);
  }

  for (u32 i = 0; i < overworld::MapGraft::GetLinkCount(); i++) {
    auto& link = overworld::MapGraft::GetLink(i);
    app.AddSection(LINK_NAMES[i])
       .Add("Leaving Map", link.from)
       .Add("Through Side", link.side)
       .WithArray(kSideNames, SIZE(kSideNames))
       .Add("Lands On Map", link.to)
       .Add("Tile X", link.tile_x)
       .WithBounds(0, WorldLayout::kMaxWidth * WorldLayout::kTilesPerBlockSide)
       .Add("Tile Z", link.tile_z)
       .WithBounds(
           0, WorldLayout::kMaxHeight * WorldLayout::kTilesPerBlockSide);
  }
}

static void PlayOverworldMusic(void*) {
  sys::Sound::PlayBackgroundMusic(
      overworld::Overworld::GetInstance().background_music);
}

namespace {
overworld::DecorationId selected_decoration =
    overworld::DecorationId::kSmallDesk;

void PlaceSelectedDecoration(void*) {
  overworld::PlacedDecorations::AddInFrontOfPlayer(selected_decoration, false);
}

void ClearDecorations(void*) { overworld::PlacedDecorations::Clear(); }
} // namespace

static void LoadDecorationPage(MainApplication& app, void* args) {
  app.Add("Decoration", selected_decoration)
     .WithArray(DECORATION_NAMES, SIZE(DECORATION_NAMES))
     .WithCallback(PlaceSelectedDecoration)
     .WithDescription("Select a decoration, then press A to place it in "
                      "front of the player.")
     .Add("Remove All", ClearDecorations)
     .WithDescription("Removes all the decorations that the menu placed.");
}

static void LoadTallGrassPage(MainApplication& app, void* args) {
  static const c8* kMixNames[] = {"All", "Green", "Ferns", "Ash"};
  auto& grass = overworld::TallGrass::GetInstance();
  app.Add("Enabled", grass.is_enabled)
     .WithDescription("Shows 3D tall grass on the tiles around the player.")
     .Add("Types", grass.mix)
     .WithArray(kMixNames, SIZE(kMixNames))
     .WithBounds(0, SIZE(kMixNames) - 1)
     .Add("Radius (tiles)", grass.radius)
     .WithBounds(1, overworld::TallGrass::kMaxRadius)
     .Add("Density (%)", grass.density)
     .WithBounds(1, 100);
}

void LoadHiddenItemPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static u32 hidden_item_index = 0;

  app.Add("Hidden Item", hidden_item_index)
     .WithBounds(0, overworld::HiddenItem::kCount - 1)
     .WithRefresh();

  auto& hidden_item = overworld::HiddenItem::GetInstance(hidden_item_index);

  app.AddSection("Item")
     .AddItem("Item", hidden_item.item_id)
     .Add("UID", hidden_item.uid)
     .AddSection("Place")
     .Add("Tile X", hidden_item.tile_x)
     .Add("Height", hidden_item.height)
     .Add("Tile Z", hidden_item.tile_z)
     .Add("Map ID", hidden_item.map_id)
     .AddSection("Respawn")
     .Add("Respawn Rate", &hidden_item.flags, 0, 7)
     .WithBounds(0, 127)
     .Add("Initially Placed", &hidden_item.flags, 7, 1);
}

void LoadRandomHiddenItemPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static u32 random_hidden_item_index = 0;

  app.Add("Random Hidden Item", random_hidden_item_index)
     .WithBounds(0, overworld::RandomHiddenItem::kCount - 1)
     .WithRefresh();

  auto& random_item = overworld::RandomHiddenItem::GetInstance(
      random_hidden_item_index);

  app.AddSection("Place")
     .Add("Map ID", random_item.map_id)
     .Add("UID", &random_item.flags, 0, 15)
     .Add("Is Mirage Spot", &random_item.flags, 15, 1)
     .Add("Tile X", random_item.tile_x)
     .Add("Height", random_item.height)
     .Add("Tile Z", random_item.tile_z)
     .AddSection("Possible Items")
     .AddItem("Item ID 0", random_item.item_id[0])
     .AddItem("Item ID 1", random_item.item_id[1])
     .AddItem("Item ID 2", random_item.item_id[2])
     .AddItem("Item ID 3", random_item.item_id[3])
     .AddItem("Item ID 4", random_item.item_id[4])
     .AddItem("Item ID 5", random_item.item_id[5]);
}

#ifdef GAME_ORAS
static void LoadWorldOverlayPage(MainApplication& app, void* args) {
  static const c8* PIVOTS[] = {"Turned (3DS)", "Not Turned"};
  auto& overlay = overworld::WorldOverlay::GetInstance();

  app.Add("Enabled", overlay.is_enabled)
     .WithDescription("Shows marks on the map when the menu is closed. Red: "
                      "wall. Blue: water. Green: wild Pokemon.")
     .AddSection("Marks")
     .Add("Tiles", overlay.show_tiles)
     .Add("Radius (tiles)", overlay.radius)
     .WithBounds(1, overworld::WorldOverlay::kMaxRadius)
     .Add("Hidden Items", overlay.show_hidden_items)
     .WithDescription("A yellow mark on each hidden item of the map.")
     .Add("Player", overlay.show_player)
     .WithDescription("A white mark on the player. If it is not on the "
                      "player, change Screen Pivot.")
     .AddSection("Advanced")
     .Add("Screen Pivot", overlay.pivot)
     .WithArray(PIVOTS, SIZE(PIVOTS))
     .WithDescription("How the camera turns the image for the screen of the "
                      "3DS.");
}
#endif

void LoadOverworldPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& map_manager = overworld::MapManager::GetInstance();
  auto& music = overworld::Overworld::GetInstance();

  app.AddSection("Map")
     .Add("Map ID", map_manager.GetMapId())
     .Add("Reload Map", RefreshMap)
     .WithDescription("Closes the menu and loads the current map again.")
     .Add("Weather", LoadWeatherPage)
     .Add("Time of Day", LoadTimeOfDayPage)
     .WithDescription("Sets a fixed time: morning, day, evening or night.")
     .AddSection("Wild Pokemon")
     .Add("Repel", CheatCodeId::kNoEncounter)
     .WithDescription("No wild Pokemon appear: an infinite Repel.")
     .Add("Encounters", LoadOverworldEncounterPage)
     .WithDescription("The walk counter, the encounter rate and the "
                      "Pokemon that move on the map.")
#ifdef GAME_ORAS
     .Add("Tall Grass", LoadTallGrassPage)
     .WithDescription("Shows 3D tall grass around the player.")
#endif
     .AddSection("Objects")
     .Add("Decorations", LoadDecorationPage)
     .WithDescription("Places Secret Base decorations in the overworld.")
     .Add("Props", LoadPropModelPage)
     .WithDescription("The doors and the other objects of the map: size, "
                      "rotation, position.")
     .Add("Hidden Items", LoadHiddenItemPage)
     .Add("Random Hidden Items", LoadRandomHiddenItemPage)
     .Add("Day Care", LoadDayCarePage)
#ifdef GAME_ORAS
     .Add("Healer Follower",
          overworld::HealerFollower::GetInstance().is_enabled)
     .WithDescription("A nurse follows the player. Talk to her to heal the "
                      "team.")
#endif
     .AddSection("Music")
     .Add("Freeze Background Music", music.freeze_background_music)
     .WithDescription("On: the overworld always plays the music below.")
     .Add("Background Music", music.background_music)
     .WithBounds(0, static_cast<u32>(BackgroundMusicId::kCount) - 1)
     .WithCallback(PlayOverworldMusic)
     .WithDescription("Press A to play this music now.")
     .AddSection("Map Editing")
#ifdef GAME_ORAS
     .Add("World Overlay", LoadWorldOverlayPage)
     .WithDescription("Shows the walls, the water, the grass and the hidden "
                      "items on the map (experimental).")
#endif
     .Add("World Layout", LoadWorldLayoutPage)
     .WithDescription("Joins maps together (grafts) and edits the tiles.")
     .Add("Map Tile", LoadOverworldMapTilePage)
     .WithDescription("Replaces the attributes of all the tiles: water, "
                      "walls, encounters...");
}
} // namespace ui