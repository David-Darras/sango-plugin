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

#pragma once
#include "common.h"
#include "hook_manager.h"
#include "utils.h"
#include "game/battle/config.h"
#include "game/constant/background.h"
#include "game/constant/background_music.h"
#include "game/constant/ball.h"
#include "game/constant/battle_format.h"
#include "game/constant/encounter_animation.h"
#include "game/constant/form.h"
#include "game/constant/gender.h"
#include "game/constant/ground.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/nature.h"
#include "game/constant/platform.h"
#include "game/constant/species.h"
#include "game/constant/weather.h"
#include "game/global_data/evolve.h"
#include "game/global_data/move.h"
#include "game/global_data/pokemon.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "parser/parser.h"
#include "parser/pokemon_node.h"
#include "ui/log_application.h"

#define ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE (0x003B0C50)

namespace feature {
struct Nuzlocke {
  MAKE_SINGLETON(Nuzlocke)
  STATIC_INLINE void Initialize() {
    FixMoves();
    FixPokemon();

    HookManager::Initialize(HookID::kLoadEvolveTable,
                            ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE,
                            (uptr)LoadEvolveTableHook
        );
  }

  static void LoadEvolveTableHook(uptr table, u16 species) {
    u16 current_species = READ(vu16, table + 4);
    HookManager::Call<void>(HookID::kLoadEvolveTable, table, species);
    if (current_species != species) {
      auto& evolve = *(global_data::Evolve*)READ(vu32, table + 8);
    }
  }

  STATIC_INLINE void FixMoves() {
    auto* move = &global_data::Move::GetInstance(0);

    move[367].base_pp = 2; // Acupressure
    move[349].base_pp = 1; // Dragon Dance
    move[14].base_pp = 1; // Swords Dance
    move[417].base_pp = 1; // Nasty Plot
    move[97].base_pp = 1; // Agility
    move[397].base_pp = 2; // Rock Polish
    move[107].base_pp = 1; // Minimize
    move[104].base_pp = 2; // Double Team
    move[339].base_pp = 2; // Bulk Up
    move[347].base_pp = 2; // Calm Mind
    move[483].base_pp = 1; // Quiver Dance
    move[133].base_pp = 1; // Amnesia
    move[294].base_pp = 1; // Tail Glow
    move[538].base_pp = 1; // Cotton Guard
    move[151].base_pp = 1; // Acid Armor
    move[254].base_pp = 2; // Stockpile
    move[174].base_pp = 2; // Curse
  }

  STATIC_INLINE void FixPokemon() {
    global_data::Pokemon* pokemon;

    // Mega-Gangar
    // Shadow Tag -> Infiltrator
    pokemon = &global_data::Pokemon::GetInstance(94, 1);
    pokemon->ability[0] = 151;
    pokemon->ability[1] = 151;
    pokemon->ability[2] = 151;

    // Blaziken
    // Speed Boost -> Blaze
    pokemon = &global_data::Pokemon::GetInstance(257, 0);
    pokemon->ability[0] = 66;
    pokemon->ability[1] = 66;
    pokemon->ability[2] = 66;

    // Mega-Blaziken
    // Speed Boost -> Quick Feet
    pokemon = &global_data::Pokemon::GetInstance(257, 1);
    pokemon->ability[0] = 95;
    pokemon->ability[1] = 95;
    pokemon->ability[2] = 95;

    // Kangaskhan
    // 125 ATK -> 100 ATK
    pokemon = &global_data::Pokemon::GetInstance(115, 0);
    pokemon->base_attack = 100;

    // Mega-Lucario
    // 130 ATK -> 145 ATK, 130 SP.ATK -> 140 SP.ATK
    pokemon = &global_data::Pokemon::GetInstance(448, 1);
    pokemon->base_attack = 145;
    pokemon->base_special_attack = 140;

    // Mega-Mawile
    // 105 ATK -> 85 ATK
    pokemon = &global_data::Pokemon::GetInstance(303, 1);
    pokemon->base_attack = 85;
  }

  STATIC_INLINE void FixTrainers(battle::Config& config, u16& trainer_id) {
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
  }
};
} // namespace feature