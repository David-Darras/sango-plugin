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
 * @file trainer_team.cc
 * @brief New teams for the trainers.
 *
 * The declarations are in battle/patch/trainer_team.h.
 */

#include "battle/patch/trainer_team.h"
#include "battle/native/config.h"
#include "core/problems.h"
#include "core/translations.h"
#include "savedata/native/pokemon_team.h"

namespace battle {

void TrainerOpponent::ApplyTo(pokemon::CoreData& pkm) const {
  pkm.Set(species, item, ability, nature, is_shiny);
  if (form != FormId::kNormal) pkm.form = form;
  pkm.SetStats(ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense,
               ev_speed);
  pkm.SetMoves(move1, move2, move3, move4);
  if (nickname != nullptr) pkm.SetNickname(nickname);
  if (forced_level != 0) pkm.SetLevel(forced_level);
  if (iv != kGameIvs) {
    pkm.iv_hp = iv;
    pkm.iv_attack = iv;
    pkm.iv_defense = iv;
    pkm.iv_speed = iv;
    pkm.iv_special_attack = iv;
    pkm.iv_special_defense = iv;
  }
}

void TrainerTeam::ApplyTo(Config& config) const {
  if (force_wild_battle_type) config.is_against_trainer = false;
  if (keeps_the_scene) {
    // The place of the game. WithWeather() can still give a weather.
    config.Set(opponent_count, format, config.background, config.ground,
               config.platform, config.encounter_animation, weather);
  } else {
    config.Set(opponent_count, format, background, ground, platform,
               encounter_animation, weather);
  }
  for (u32 i = 0; i < opponent_count; i++) {
    opponents[i].ApplyTo(config.GetOpponent(i));
  }

  // The trainers on the side of the opponent: the clients 1 and 3.
  static const u32 kOpponentClients[] = {1, 3};
  for (u32 client : kOpponentClients) {
    TrainerData* trainer = config.trainer_data[client];
    if (trainer == nullptr) continue;
    if (has_ai) trainer->ai_flags = ai;
    if (has_items) {
      for (u32 i = 0; i < 4; i++) trainer->items[i] = items[i];
    }
    if (money != 0) trainer->money_calculation_coefficient = money;
    if (name != nullptr && trainer->name != nullptr) {
      trainer->name->Set(core::Translate(name));
    }
  }
}

bool TrainerTeams::Add(TrainerId id, const TrainerTeam* team) {
  auto& ctx = GetInstance();
  if (team == nullptr) return false;
  if (ctx.teams_.IsFull()) {
    core::Problems::Add(u"trainer %u: more than %u teams (use SetTable)",
                        static_cast<u32>(id), kMaxTeams);
    return false;
  }
  ctx.teams_.Add({id, team});
  return true;
}

void TrainerTeams::SetTable(const TrainerTeamEntry* table, u32 count) {
  auto& ctx = GetInstance();
  ctx.table_ = table;
  ctx.table_count_ = count;
}

void TrainerTeams::Clear() {
  auto& ctx = GetInstance();
  ctx.teams_.Clear();
  ctx.table_ = nullptr;
  ctx.table_count_ = 0;
}

const TrainerTeam* TrainerTeams::Find(TrainerId id) {
  auto& ctx = GetInstance();
  for (const TrainerTeamEntry& entry : ctx.teams_) {
    if (entry.id == id) return entry.team;
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

  {
    // ApplyTo() changes all the Pokemon of the team at the same time.
    pokemon::EditTeam edit(opponents->pokemons, count);
    team->ApplyTo(config);
    for (u32 i = 0; i < edit.Count(); i++) edit[i].ResetNickname();
  }  // Encrypts the team, then calculates the stats again.
  opponents->HealAllPokemons();
}

} // namespace battle
