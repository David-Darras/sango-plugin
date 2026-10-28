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
#include "overworld/patch/wild_encounter.h"
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
#include "ui/log.h"
#include "ui/main_application.h"
#include "ui/page/enum_names.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"
#include "ui/tile_editor_application.h"
#include "overworld/address.h"

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
       .WithDescription("Select a weather, then press {A} to show it.");
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
          [](void*) { overworld::TileEditor::ClearEdits(); })
     .WithConfirm()
     .WithDescription("Removes all the tile edits. Hold {A} to confirm.");

  for (u32 i = 0; i < overworld::MapGraft::GetCount(); i++) {
    auto& request = overworld::MapGraft::GetRequest(i);
    auto& result = overworld::MapGraft::GetResult(i);
    app.AddSection(GRAFT_NAMES[i])
       .Add("Is Applied", result.is_applied)
       .WithReadOnly()
       .Add("Anchor Map", request.anchor)
       .WithArray(kMapNames, SIZE(kMapNames))
       .Add("Side", request.side)
       .WithArray(kSideNames, SIZE(kSideNames))
       .Add("Grafted Map", request.map)
       .WithArray(kMapNames, SIZE(kMapNames))
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
       .WithArray(kMapNames, SIZE(kMapNames))
       .Add("Through Side", link.side)
       .WithArray(kSideNames, SIZE(kSideNames))
       .Add("Lands On Map", link.to)
       .WithArray(kMapNames, SIZE(kMapNames))
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
     .WithDescription("Select a decoration, then press {A} to place it in "
         "front of the player.")
     .Add("Remove All", ClearDecorations)
     .WithConfirm()
     .WithDescription("Removes all the decorations that the menu placed. "
         "Hold {A} to confirm.");
}

void LoadTallGrassPage(MainApplication& app, void* args) {
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
     .Add("Tile Z", hidden_item.tile_z)
     .Add("Height", hidden_item.height)
     .Add("Map ID", hidden_item.map_id)
     .WithArray(kMapNames, SIZE(kMapNames))
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
     .WithArray(kMapNames, SIZE(kMapNames))
     .Add("UID", &random_item.flags, 0, 15)
     .Add("Is Mirage Spot", &random_item.flags, 15, 1)
     .Add("Tile X", random_item.tile_x)
     .Add("Tile Z", random_item.tile_z)
     .Add("Height", random_item.height)
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
     .WithDescription("Shows marks on the map when the menu is closed. Turn "
         "on the Legend to see the colors.")
     .AddSection("Tiles")
     .Add("Walls, Water, Wild Pokemon", overlay.show_tiles)
     .WithDescription("Red: wall. Blue: water. Green: wild Pokemon.")
     .Add("Blocks Trainers", overlay.show_trainer_blockers)
     .WithDescription("Orange: the tile blocks the trainers.")
     .Add("Decoration Spots", overlay.show_decoration_spots)
     .WithDescription("Purple: a decoration can stand on the tile.")
     .Add("Fill In Perspective", overlay.fill_tiles)
     .WithDescription("On: each tile is filled like the ground. Off: a small "
         "square on each tile (fewer drawings).")
     .Add("Radius (tiles)", overlay.radius)
     .WithBounds(1, overworld::WorldOverlay::kMaxRadius)
     .Add("Opacity (%)", overlay.opacity)
     .WithBounds(10, 100)
     .AddSection("Events")
     .Add("Signs", overlay.show_signs)
     .WithDescription("Cyan: the zone where {A} starts a script.")
     .Add("Triggers", overlay.show_triggers)
     .WithDescription("Magenta: a script starts when you walk in. Pink: when "
         "you push against the zone.")
     .Add("Warps", overlay.show_warps)
     .WithDescription("Lime: the zones that change the map.")
     .Add("Hidden Items", overlay.show_hidden_items)
     .WithDescription("A yellow mark on each hidden item of the map.")
     .Add("Script Numbers", overlay.show_script_numbers)
     .WithDescription("Writes the number of the script over the zones and "
         "the characters (20 at most).")
     .AddSection("Characters")
     .Add("Characters", overlay.show_characters)
     .WithDescription("A square on each character. Orange: a trainer. Blue: "
         "another character.")
     .Add("Trainer Sight", overlay.show_trainer_sight)
     .WithDescription("Orange tiles: where the trainer sees the player. The "
         "line stops at the first wall.")
     .AddSection("Display")
     .Add("Player", overlay.show_player)
     .WithDescription("A white mark on the player. If it is not on the "
         "player, change Screen Pivot.")
     .Add("Legend", overlay.show_legend)
     .WithDescription("The list of the colors, at the bottom left.")
     .AddSection("State")
     .Add("Frames With A Camera", overlay.camera_frames)
     .WithReadOnly()
     .WithDescription("0 or not increasing: the overlay does not find the "
         "camera of the overworld.")
     .Add("Marks Of The Last Frame", overlay.marks_drawn)
     .WithReadOnly()
     .WithDescription("0 with a camera: the points are out of the screen "
         "(try the other Screen Pivot).")
     .Add("Strips Of The Last Frame", overlay.strips_drawn)
     .WithReadOnly()
     .WithDescription("The thin rectangles that fill the tiles. Many strips "
         "slow the game: lower the radius.")
     .Add("Scene Draws Seen", overlay.draws_seen)
     .WithReadOnly()
     .WithDescription("How many times the game drew a scene since the start. "
         "0: the hooks of the overlay are not called.")
     .Add("Top Cameras Found", overlay.cameras_found)
     .WithReadOnly()
     .WithDescription("The cameras with the shape of the top screen. 0 with "
         "draws: the overlay refuses all the cameras.")
     .Add("Cameras Refused", overlay.cameras_refused)
     .WithReadOnly()
     .WithDescription("The cameras that do not have the shape of the top "
         "screen (the bottom screen, the effects).")
     .AddSection("Advanced")
     .Add("Screen Pivot", overlay.pivot)
     .WithArray(PIVOTS, SIZE(PIVOTS))
     .WithDescription("How the camera turns the image for the screen of the "
         "3DS.");
}
#endif

namespace {
u32 target_map = 0;
// The destination in tiles, like the place of the player (Tile X, Tile Z,
// Height).
Vec3 target_tile = Vec3(20.0f, 0.0f, 20.0f);
// A tile is 18 x 18 units in the world; the step of the height is 9 units.
constexpr f32 kUnitsPerTile = 18.0f;
constexpr f32 kUnitsPerHeight = 9.0f;
bool is_target_set = false;

// Takes the map and the place of the player as the destination.
void TakeCurrentPlace() {
  target_map = static_cast<u32>(overworld::MapManager::GetInstance().GetMap());
  target_tile =
      overworld::ModelManager::GetInstance().GetPlayer().map_pos.coords;
}

// The entry "Use My Place": the page builds itself again with the new values.
void UseCurrentPlace(void*) {
  TakeCurrentPlace();
  MainApplication::GetInstance().Refresh();
}

// Sends the player to the destination.
void Teleport(void*) {
  auto& main_app = MainApplication::GetInstance();
  if (main_app.CheckProcess(overworld::address::kVtable)) return;

  // Keeps the other fields of the position of the player (scale, rotation) and
  // changes only the coordinates. The game wants the world: the center of the
  // tile, in units.
  overworld::Position position =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  position.coords.x = (target_tile.x + 0.5f) * kUnitsPerTile;
  position.coords.y = target_tile.y * kUnitsPerHeight;
  position.coords.z = (target_tile.z + 0.5f) * kUnitsPerTile;
  overworld::MapManager::ChangeMap(static_cast<overworld::MapId>(target_map),
                                   position, overworld::Facing::kUp, false,
                                   false);
  main_app.ForceClose();
}
} // namespace

void LoadLocationPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  // The first time, the destination is the place of the player.
  if (!is_target_set) {
    is_target_set = true;
    // Not UseCurrentPlace(): a Refresh() inside the loading of the page would
    // load the page a second time, and the page would have all its entries
    // twice.
    TakeCurrentPlace();
  }

  auto& map_manager = overworld::MapManager::GetInstance();
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();

  app.AddSection("Current Place")
     .Add("Reload Map", RefreshMap)
     .WithDescription("Closes the menu and loads the current map again.")
     .Add("Map", map_manager.GetMapId())
     .WithArray(kMapNames, SIZE(kMapNames))
     .WithReadOnly()
     .WithDescription("The map where the player is now.")
     .Add("Map ID", map_manager.GetMapId())
     .WithReadOnly()
     .Add("Tile X", player.map_pos.coords.x)
     .WithReadOnly()
     .Add("Tile Z", player.map_pos.coords.z)
     .WithReadOnly()
     .Add("Height", player.map_pos.coords.y)
     .WithReadOnly()
     .Add("World X", player.world_pos.coords.x)
     .WithReadOnly()
     .Add("World Z", player.world_pos.coords.z)
     .WithReadOnly()
     .Add("World Height", player.world_pos.coords.y)
     .WithReadOnly()
     .WithDescription("The position of the player in world units. A block "
         "has 720 x 720 units.")
     .AddSection("Teleport To")
     .Add("Map", target_map)
     .WithArray(kMapNames, SIZE(kMapNames))
     .WithCallback(Teleport)
     .WithDescription("Press {A} to teleport to this map and to the tile "
         "below.")
     .Add("Tile X", target_tile.x)
     .Add("Tile Z", target_tile.z)
     .Add("Height", target_tile.y)
     .Add("Use My Place", UseCurrentPlace)
     .WithDescription("The destination becomes the map and the place of the "
         "player now.");
}

namespace {
const c8* kEncounterModeNames[] = {"Normal", "Never", "Always"};
} // namespace

void LoadOverworldPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& music = overworld::Overworld::GetInstance();

  app.AddSection("Actions")
     .Add("Applications", LoadAppPage)
     .WithDescription("Opens the PC, the Move Reminder or the Move Deleter "
         "from anywhere.")
     .Add("Field Moves", LoadOverworldFieldMovePage)
     .WithDescription("Uses Surf, Cut, Fly... without a Pokemon that knows "
         "the move.")
     .Add("Location", LoadLocationPage)
     .WithDescription("The map and the place of the player, and the "
         "teleport to any map.")
     .AddSection("Environment")
     .Add("Weather", LoadWeatherPage)
     .Add("Time of Day", LoadTimeOfDayPage)
     .WithDescription("Sets a fixed time: morning, day, evening or night.")
     .AddSection("Wild Pokemon")
     .Add("Wild Encounters", overworld::WildEncounter::GetInstance().mode)
     .WithArray(kEncounterModeNames, SIZE(kEncounterModeNames))
     .WithDescription("Normal: the rule of the game. Never: no wild Pokemon "
         "appear (better than a Repel). Always: a battle at "
         "each step in the grass.")
     .Add("Encounter Counters", LoadOverworldEncounterPage)
     .WithDescription("The walk counter, the encounter rate and the "
         "Pokemon that move on the map.")
     .AddSection("Scene")
     .Add("Characters", LoadCharactersPage)
     .WithDescription("The player and the people of the map: model, "
         "animation, position and size.")
     .Add("Camera", LoadOverworldCameraPage)
     .Add("Map Events", LoadMapEventsPage)
     .WithDescription("The signs and the triggers of the map: the zones that "
         "start a script. Change them or add your own.")
     .AddSection("Music")
     .Add("Freeze Background Music", music.freeze_background_music)
     .WithDescription("On: the overworld always plays the music below.")
     .Add("Background Music", music.background_music)
     .WithArray(kBackgroundMusicNames, SIZE(kBackgroundMusicNames))
     .WithCallback(PlayOverworldMusic)
     .WithDescription("Press {A} to play this music now.")
     .AddSection("Objects")
     .Add("Decorations", LoadDecorationPage)
     .WithDescription("Places Secret Base decorations in the overworld.")
     .Add("Props", LoadPropModelPage)
     .WithDescription("The doors and the other objects of the map: size, "
         "rotation, position.")
     .AddSection("Map Editing")
     .Add("Hidden Items", LoadHiddenItemPage)
     .Add("Random Hidden Items", LoadRandomHiddenItemPage)
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