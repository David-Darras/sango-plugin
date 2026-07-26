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

#include "utils.h"
#include "game/battle/config.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "parser/pokemon_node.h"
#include "parser/pokemon_showdown_parser.h"
#include "ui/log_application.h"

namespace kaizo {
void PatchTrainerData(battle::Config& config, u16& trainer_id) {
  ui::LogApplication::Print(u"[%u] %ls %ls wants to battle!", trainer_id,
                            config.trainer_data[1]->name->GetBuffer(),
                            config.trainer_data[1]->title_name->GetBuffer());

  parser::AST ast;
  c16 buffer[BUFFER_SIZE];

  Utils::Format(
      buffer, u"sdmc:/luma/plugins/000400000011C500/trainers/%u.txt",
      trainer_id);
  bool res = parser::ParsePokemonShowdownFile(buffer, ast);
  if (!res) {
    ui::LogApplication::Print(u"Error");
    return;
  }

  config.pokemon_teams[1]->count = ast.size();

  for (u32 i = 1; i < 6; i++) {
    *config.pokemon_teams[1]->pokemons[i]->core = *config.pokemon_teams[1]->
        pokemons[0]->core;
    *config.pokemon_teams[1]->pokemons[i]->runtime = *config.pokemon_teams[1]
        ->pokemons[0]->runtime;
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->accessor->Decrypt();
  }

  u32 index = 0;
  for (const auto& node : ast) {
    auto& pkm = *config.pokemon_teams[1]->pokemons[index++]->core;

    pkm.species = node.species;
    if (node.has_nickname) {
      pkm.SetNickname(node.nickname);
    } else {
      pkm.ResetNickname();
    }
    pkm.gender = node.gender;
    pkm.item = node.item;
    pkm.ability = node.ability;
    pkm.SetShiny(node.is_shiny);
    pkm.ball = node.ball;
    pkm.nature = node.nature;
    pkm.SetLevel(node.level);
    pkm.contest_friendship = pkm.happiness = node.happiness;

    pkm.moves[0] = node.moves[0];
    pkm.moves[1] = node.moves[1];
    pkm.moves[2] = node.moves[2];
    pkm.moves[3] = node.moves[3];

    pkm.ev_hp = node.evs.hp;
    pkm.ev_attack = node.evs.atk;
    pkm.ev_defense = node.evs.def;
    pkm.ev_speed = node.evs.spe;
    pkm.ev_special_attack = node.evs.spa;
    pkm.ev_special_defense = node.evs.spd;

    pkm.iv_hp = node.ivs.hp;
    pkm.iv_attack = node.ivs.atk;
    pkm.iv_defense = node.ivs.def;
    pkm.iv_speed = node.ivs.spe;
    pkm.iv_special_attack = node.ivs.spa;
    pkm.iv_special_defense = node.ivs.spd;
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->accessor->Encrypt();
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->UpdateRuntimeData();
  }

  config.pokemon_teams[1]->HealAllPokemons();

  // Best AI
  if (config.trainer_data[0] != nullptr)
    config.trainer_data[0]->ai_flags = 0b111;
  if (config.trainer_data[1] != nullptr)
    config.trainer_data[1]->ai_flags = 0b111;
  if (config.trainer_data[2] != nullptr)
    config.trainer_data[2]->ai_flags = 0b111;
  if (config.trainer_data[3] != nullptr)
    config.trainer_data[3]->ai_flags = 0b111;
}
}