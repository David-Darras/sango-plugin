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

#include "feature/feature_model_loader.h"
#include "game/data_manager.h"
#include "game/data_manager.h"
#include "game/constant/model.h"
#include "game/constant/species.h"
#include "game/overworld/model_manager.h"
#include "ui/main_application.h"
#include "ui/page/page_top.h"
#include "game/savedata/pokemon_team.h"

feature::LoadedModel g_my_1st_pokemon;

namespace {
struct Settings {
  u16 model_id = MODEL_STEVEN_STONE;
  u16 species = 317;
  u16 form = 0;
  bool is_shiny = true;
  f32 scale = 1.0f;
  f32 distance = 0.0f;
};

Settings& GetSettings() {
  static Settings settings;
  return settings;
}

feature::LoadedModel g_overworld_model;
feature::LoadedModel g_pokemon_model;


Vec3 SpawnPosition() {
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  Vec3 position = player.GetDrawModel().position;
  position.x += GetSettings().distance;
  return position;
}

void ApplyScale(const feature::LoadedModel& loaded, f32 scale) {
  if (loaded.model == nullptr || scale == 1.0f) return;
  loaded.model->SetScale(scale, scale, scale);
}

void SpawnOverworldModel(void*) {
  if (g_overworld_model.IsLoaded()) {
    ui::LogApplication::Print(u"already spawned, clear first");
    return;
  }
  const u16 model_id = GetSettings().model_id;
  if (feature::ModelLoader::LoadOverworldModel(&g_overworld_model, model_id,
                                               SpawnPosition())) {
    ApplyScale(g_overworld_model, GetSettings().scale);
    ui::LogApplication::Print(u"model %d spawned", model_id);
  }
}

void SpawnPokemon(void*) {
  if (g_pokemon_model.IsLoaded()) {
    ui::LogApplication::Print(u"already spawned, clear first");
    return;
  }
  const Settings& settings = GetSettings();
  if (feature::ModelLoader::LoadPokemon(&g_pokemon_model, settings.species,
                                        (u8)settings.form, settings.is_shiny,
                                        SpawnPosition())) {
    ApplyScale(g_pokemon_model, GetSettings().scale);
    ui::LogApplication::Print(u"species %d spawned", settings.species);
  }
}

void Spawn1stPokemon(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  auto& pkm = *team.pokemons[0];
  pkm.accessor->Decrypt();
  u16 species = pkm.core->species;
  u16 form = pkm.core->form;
  bool is_shiny = PokemonUtils::IsShiny(pkm.core->id, pkm.core->shiny_id);
  if (feature::ModelLoader::LoadPokemon(&g_my_1st_pokemon, species,
                                        (u8)form, is_shiny,
                                        SpawnPosition())) {
    ApplyScale(g_my_1st_pokemon, 0.25f);
  }
}

void ClearModels(void*) { feature::ModelLoader::DropAll(); }
} // namespace

namespace ui {
void LoadModelLoaderPage(MainApplication& app, void* args) {
  Settings& settings = GetSettings();

  app.Add("Spawn 1st Pokémon", Spawn1stPokemon)
     .Add("Spawn model", SpawnOverworldModel)
     .Add("Model", settings.model_id)
     .WithBounds(0, 0x1FF)
     .AddSeparator()
     .Add("Spawn Pokemon", SpawnPokemon)
     .AddSpecies("Species", settings.species)
     .WithBounds(1, SPECIES_VOLCANION)
     .Add("Form", settings.form)
     .WithBounds(0, 30)
     .Add("Shiny", settings.is_shiny)
     .AddSeparator()
     .Add("Scale", settings.scale)
     .WithFactor(0.1f)
     .Add("Distance", settings.distance)
     .WithFactor(1.0f);
  //.AddSeparator()
  //.Add("Clear spawned", ClearModels);
}
} // namespace ui

void UpdateFollowingPokemon() {
  if (g_my_1st_pokemon.model != nullptr &&
      game::ProcessManager::GetInstance().IsCurrentProcess(
          ADDRESS_OVERWORLD_VTABLE)) {
    auto& player = overworld::ModelManager::GetInstance().GetPlayer();
    auto& model = player.GetDrawModel();
    Vec3 pos = model.position;
    Vec3 rot = model.rotation;
    pos.x += -16.0 * player.facing_direction.x;
    pos.y += -16.0 * player.facing_direction.y;
    pos.z += -16.0 * player.facing_direction.z;
    g_my_1st_pokemon.model->SetTranslate(pos);
    g_my_1st_pokemon.model->SetRotate(rot);
  }
}