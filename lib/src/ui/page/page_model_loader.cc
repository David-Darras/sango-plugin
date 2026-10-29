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
 * @file page_model_loader.cc
 * @brief The menu page that loads 3D models in the overworld.
 */

#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "overworld/constant/model.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include <cstring>

#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"
#include "pokemon/patch/following_pokemon.h"
#include "pokemon/patch/model_loader.h"
#include "renderer/native/h3d_shader_model.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/data_accessor.h"
#include "savedata/native/pokemon_team.h"
#include "script/native/engine.h"
#include "system/native/controller.h"
#include "ui/log.h"
#include "ui/main_application.h"
#include "ui/page/enum_names.h"
#include "ui/page/pages.h"
#include "overworld/address.h"

pokemon::LoadedModel g_my_1st_pokemon;

namespace {
struct Settings {
  ModelId model_id = GAME_CONSTANT(ModelId::kProfessorSycamore, ModelId::kStevenStone);
  SpeciesId species = SpeciesId::kSwalot;
  FormId form = FormId::kNormal;
  bool is_shiny = true;
  f32 scale = 1.0f;
  f32 distance = 0.0f;
};

Settings& GetSettings() {
  static Settings settings;
  return settings;
}

pokemon::LoadedModel g_overworld_model;
pokemon::LoadedModel g_pokemon_model;


Vec3 SpawnPosition() {
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  overworld::DrawModel* draw = player.GetDrawModelOrNull();
  Vec3 position = draw != nullptr ? draw->position : Vec3();
  position.x += GetSettings().distance;
  return position;
}

void ApplyScale(const pokemon::LoadedModel& loaded, f32 scale) {
  if (loaded.model == nullptr || scale == 1.0f) return;
  loaded.model->SetScale(scale, scale, scale);
}

void SpawnOverworldModel(void*) {
  // A new model replaces the model that is there.
  pokemon::ModelLoader::Drop(&g_overworld_model);
  const ModelId model_id = GetSettings().model_id;
  if (pokemon::ModelLoader::LoadOverworldModel(&g_overworld_model, model_id,
                                               SpawnPosition())) {
    ApplyScale(g_overworld_model, GetSettings().scale);
    ui::Log::Print(u"model %d spawned", model_id);
  }
}

void SpawnPokemon(void*) {
  pokemon::ModelLoader::Drop(&g_pokemon_model);
  const Settings& settings = GetSettings();
  if (pokemon::ModelLoader::LoadPokemon(&g_pokemon_model, settings.species,
                                        settings.form, settings.is_shiny,
                                        SpawnPosition())) {
    ApplyScale(g_pokemon_model, GetSettings().scale);
    ui::Log::Print(u"species %d spawned", settings.species);
  }
}

bool g_wants_follower = false; // The player asked for the Pokémon that follows.
u32 g_frame = 0;
u32 g_ready_frame = 0; // The Pokémon appears after this frame.
u32 g_last_map = 0xFFFFFFFF;
// The Pokémon waits some frames for the end of a loading.
constexpr u32 kReadyDelayFrames = 20;

// Makes the model of the first Pokémon of the team. Returns true on success.
bool SpawnFollower() {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return false;
  auto& pkm = *team.pokemons[0];

  // Read the Pokémon of the team without changing it. The data of the team is
  // encrypted: decrypting it and leaving it decrypted breaks the Pokémon (the
  // next decryption mixes the data again). The code keeps a copy of the
  // encrypted data, and puts it back after the reading.
  pokemon::CoreData backup;
  std::memcpy(&backup, pkm.core, sizeof(backup));
  pkm.accessor->Decrypt();
  const SpeciesId species = pkm.core->species;
  const FormId form = pkm.core->form;
  const bool is_shiny =
      pokemon::Utils::IsShiny(pkm.core->id, pkm.core->shiny_id);
  pkm.accessor->Encrypt();
  std::memcpy(pkm.core, &backup, sizeof(backup));
  if (species == SpeciesId::kNone || species >= SpeciesId::kCount) return false;

  if (!pokemon::ModelLoader::LoadPokemon(&g_my_1st_pokemon, species, form,
                                         is_shiny, SpawnPosition())) {
    return false;
  }
  ApplyScale(g_my_1st_pokemon, 0.25f);
  pokemon::FollowingPokemon::Reset();
  return true;
}

void Spawn1stPokemon(void*) {
  g_wants_follower = true;
  if (!g_my_1st_pokemon.IsLoaded()) SpawnFollower();
}

void ClearModels(void*) {
  g_wants_follower = false;
  pokemon::ModelLoader::DropAll();
}
} // namespace

namespace ui {
void LoadModelLoaderPage(MainApplication& app, void* args) {
  Settings& settings = GetSettings();

  app.AddSection("Following Pokemon")
     .Add("Spawn 1st Pokemon", Spawn1stPokemon)
     .WithDescription("The first Pokemon of the team follows the player.")
     .AddSection("Character")
     .Add("Spawn Model", SpawnOverworldModel)
     .Add("Model", settings.model_id)
     .WithArray(kModelNames, SIZE(kModelNames))
     .WithBounds(0, static_cast<u32>(ModelId::kLastCharacter))
     .AddSection("Pokemon")
     .Add("Spawn Pokemon", SpawnPokemon)
     .AddSpecies("Species", settings.species)
     .WithBounds(1, static_cast<u32>(SpeciesId::kCount) - 1)
     .Add("FormId", settings.form)
     .WithBounds(0, 30)
     .Add("Shiny", settings.is_shiny)
     .AddSection("Place")
     .Add("Clear Models", ClearModels)
     .WithDescription("Removes the models of this page.")
     .Add("Scale", settings.scale)
     .WithFactor(0.1f)
     .Add("Distance", settings.distance)
     .WithFactor(1.0f);
}
} // namespace ui

bool& FollowingPokemonSwitch() { return g_wants_follower; }

void ApplyFollowingPokemon(void*) {
  if (g_wants_follower) return;
  // The switch is off: the Pokémon leaves the scene. The menu of the plugin is
  // open over the overworld, so the scene is still there.
  if (g_my_1st_pokemon.IsLoaded()) pokemon::ModelLoader::Drop(&g_my_1st_pokemon);
  pokemon::FollowingPokemon::Reset();
}

void RemoveFollowingPokemonBeforeBattle() {
  // The scene is still there: the model leaves it as it should.
  if (g_my_1st_pokemon.IsLoaded()) pokemon::ModelLoader::Drop(&g_my_1st_pokemon);
  pokemon::FollowingPokemon::Reset();
  g_ready_frame = g_frame + kReadyDelayFrames;
}

// Writes a step of the state of the Pokémon one time, when the state changes.
void TraceState(u32 state) {
  static u32 last_state = 0xFFFF;
  if (state == last_state) return;
  last_state = state;
  pokemon::ModelLoader::TraceStep("state", state, g_frame);
}

void UpdateFollowingPokemon() {
  g_frame++;

  // The game destroys the scene of the overworld in a battle, in a menu and
  // when it loads a map. A model that stays in a destroyed scene is a crash.
  if (!core::ProcessManager::IsOverworldActive()) {
    TraceState(1);
    pokemon::ModelLoader::Discard(&g_my_1st_pokemon);
    pokemon::ModelLoader::Discard(&g_pokemon_model);
    pokemon::ModelLoader::Discard(&g_overworld_model);
    // The game unloads the overworld for a menu: its heaps are not the same
    // after the return.
    pokemon::ModelLoader::ForgetGameMemory();
    pokemon::FollowingPokemon::Reset();
    g_ready_frame = g_frame + kReadyDelayFrames;
    g_last_map = 0xFFFFFFFF;
    return;
  }
  // The manager of the maps does not exist in the first frames.
  overworld::MapManager* maps_pointer = overworld::MapManager::GetInstanceOrNull();
  if (maps_pointer == nullptr) return;
  auto& maps = *maps_pointer;
  if (maps.GetNextMapId() != overworld::MapManager::kNoMap) {
    TraceState(3);
    // A new map starts to load: the scene is still there for some frames.
    pokemon::ModelLoader::Drop(&g_my_1st_pokemon);
    pokemon::ModelLoader::Drop(&g_pokemon_model);
    pokemon::ModelLoader::Drop(&g_overworld_model);
    pokemon::ModelLoader::ForgetGameMemory();
    pokemon::FollowingPokemon::Reset();
    g_ready_frame = g_frame + kReadyDelayFrames;
    return;
  }
  const u32 map = static_cast<u32>(maps.GetMap());
  if (map != g_last_map) {
    TraceState(4);
    g_last_map = map;
    pokemon::ModelLoader::ForgetGameMemory();
    pokemon::ModelLoader::Discard(&g_my_1st_pokemon);
    pokemon::ModelLoader::Discard(&g_pokemon_model);
    pokemon::ModelLoader::Discard(&g_overworld_model);
    pokemon::FollowingPokemon::Reset();
    g_ready_frame = g_frame + kReadyDelayFrames;
    return;
  }
  // A script or an event of the game runs (a talk, a cutscene, a door): the
  // Pokémon leaves the scene and comes back after it. A door is an event
  // without script: the game freezes if the model stays in the scene.
  auto* engine = script::Engine::GetInstance();
  const bool is_script_running =
      (engine != nullptr && engine->IsScriptRunning()) ||
      core::EventManager::GetInstance().GetGameEvent() != nullptr;
  if (is_script_running) {
    TraceState(5);
    pokemon::ModelLoader::Drop(&g_my_1st_pokemon);
    pokemon::FollowingPokemon::Reset();
    g_ready_frame = g_frame + kReadyDelayFrames;
    return;
  }
  if (g_wants_follower && g_frame >= g_ready_frame &&
      !g_my_1st_pokemon.IsLoaded()) {
    pokemon::ModelLoader::TraceStep("spawn", g_frame);
    if (!SpawnFollower()) g_ready_frame = g_frame + 60;
  }
  if (!g_my_1st_pokemon.IsLoaded()) return;

  pokemon::FollowingPokemon::Update(&g_my_1st_pokemon);

  // L: an animation of the battles. R: an animation of Pokémon-Amie. The
  // Pokémon also cries.
  auto& controller = sys::Controller::GetInstance();
  if (controller.IsKeyPressed(Key::kL)) {
    pokemon::ModelLoader::Act(&g_my_1st_pokemon, false);
  } else if (controller.IsKeyPressed(Key::kR)) {
    pokemon::ModelLoader::Act(&g_my_1st_pokemon, true);
  }
  pokemon::ModelLoader::UpdateMotions();
}
