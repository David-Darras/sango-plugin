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

#include "battle/patch/trainer_team.h"
#include "battle/native/config.h"
#include "pokemon/native/global_data/pokemon.h"
#include "pokemon/native/data_accessor.h"
#include "savedata/native/pokemon_team.h"

namespace battle {

void TrainerOpponent::ApplyTo(pokemon::CoreData& pkm) const {
  pkm.Set(species, item, ability, nature, is_shiny);
  if (form != Form::kNormal) pkm.form = form;
  pkm.SetStats(ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense,
               ev_speed);
  pkm.SetMoves(move1, move2, move3, move4);
  if (nickname != nullptr) pkm.SetNickname(nickname);
  if (forced_level != 0) pkm.SetLevel(forced_level);
}

void TrainerTeam::ApplyTo(Config& config) const {
  if (force_wild_battle_type) config.is_against_trainer = false;
  if (keeps_the_scene) {
    config.Set(opponent_count, format, config.background, config.ground,
               config.platform, config.encounter_animation,
               Weather::kInvalid);
  } else {
    config.Set(opponent_count, format, background, ground, platform,
               encounter_animation, weather);
  }
  for (u32 i = 0; i < opponent_count; i++) {
    opponents[i].ApplyTo(config.GetOpponent(i));
  }
}

bool TrainerTeams::Add(TrainerId id, const TrainerTeam* team) {
  auto& ctx = GetInstance();
  if (ctx.count_ >= kMaxTeams || team == nullptr) return false;
  ctx.teams_[ctx.count_].id = id;
  ctx.teams_[ctx.count_].team = team;
  ctx.count_++;
  return true;
}

void TrainerTeams::SetTable(const TrainerTeamEntry* table, u32 count) {
  auto& ctx = GetInstance();
  ctx.table_ = table;
  ctx.table_count_ = count;
}

void TrainerTeams::Clear() {
  auto& ctx = GetInstance();
  ctx.count_ = 0;
  ctx.table_ = nullptr;
  ctx.table_count_ = 0;
}

const TrainerTeam* TrainerTeams::Find(TrainerId id) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.count_; i++) {
    if (ctx.teams_[i].id == id) return ctx.teams_[i].team;
  }
  for (u32 i = 0; i < ctx.table_count_; i++) {
    if (ctx.table_[i].id == id) return ctx.table_[i].team;
  }
  return nullptr;
}

void TrainerTeams::Apply(Config& config, TrainerId trainer_id) {
  const TrainerTeam* team = Find(trainer_id);
  if (team == nullptr) return;

  savedata::PokemonTeam* opponents = config.pokemon_teams[1];
  if (opponents == nullptr) return;

  u32 count = team->opponent_count;
  if (count > 6) count = 6;

  for (u32 i = 1; i < count; i++) {
    *opponents->pokemons[i]->core = *opponents->pokemons[0]->core;
    *opponents->pokemons[i]->runtime = *opponents->pokemons[0]->runtime;
  }

  for (u32 i = 0; i < count; i++) {
    opponents->pokemons[i]->accessor->Decrypt();
  }

  team->ApplyTo(config);

  for (u32 i = 0; i < count; i++) {
    opponents->pokemons[i]->core->ResetNickname();
    opponents->pokemons[i]->accessor->Encrypt();
  }
  for (u32 i = 0; i < count; i++) {
    opponents->pokemons[i]->UpdateRuntimeData();
  }
  opponents->HealAllPokemons();
}

} // namespace battle
