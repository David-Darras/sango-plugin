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
    for (u32 i = 0; i < config.pokemon_teams[1]->count; i++) {
      config.pokemon_teams[1]->pokemons[i]->accessor->Decrypt();
    }

    auto& pkm1 = *config.pokemon_teams[1]->pokemons[0]->core;
    auto& pkm2 = *config.pokemon_teams[1]->pokemons[1]->core;
    auto& pkm3 = *config.pokemon_teams[1]->pokemons[2]->core;
    auto& pkm4 = *config.pokemon_teams[1]->pokemons[3]->core;
    auto& pkm5 = *config.pokemon_teams[1]->pokemons[4]->core;
    auto& pkm6 = *config.pokemon_teams[1]->pokemons[5]->core;

    switch (trainer_id) {
      case 7: // Youngster
      {
        config.battle_format = BATTLE_FORMAT_SINGLE;
        config.ground = GROUND_ZINNIA_LAST_BATTLE;
        config.background = BACKGROUND_FOREST;
        config.platform = PLATFORM_ELITE_FOUR_GLACIA;
        config.encounter_animation = ENCOUNTER_ANIMATION_WILD_HOOPA_RING;
        config.background_music = (1 << 16) + BACKGROUND_MUSIC_POKEMON_THEME;
        // config.weather = WEATHER_HEAVY_RAIN;

        pkm1.species = SPECIES_ALAKAZAM;
        pkm1.SetNickname(u"ZettaD");
        pkm1.SetLevel(100);
        pkm1.item = ITEM_LIFE_ORB;
        pkm1.nature = NATURE_TIMID;
        pkm1.gender = GENDER_FEMALE;
        pkm1.form = FORM_ALAKAZAM_MEGA;
        pkm1.SetShiny(true);
        pkm1.ball = BALL_MASTER_BALL;

        pkm1.moves[0] = MOVE_LIGHT_OF_RUIN;
        pkm1.pp[0] = 99;
        pkm1.moves[1] = MOVE_DRAGON_ASCENT;
        pkm1.pp[1] = 99;
        pkm1.moves[2] = MOVE_PRECIPICE_BLADES;
        pkm1.pp[2] = 99;
        pkm1.moves[3] = MOVE_ORIGIN_PULSE;
        pkm1.pp[3] = 99;

        pkm1.ev_hp = 255;
        pkm1.ev_attack = 255;
        pkm1.ev_defense = 255;
        pkm1.ev_speed = 255;
        pkm1.ev_special_attack = 255;
        pkm1.ev_special_defense = 255;

        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
        pkm1.iv_attack = 31;
      }
      break;
    }

    for (u32 i = 0; i < config.pokemon_teams[1]->count; i++) {
      config.pokemon_teams[1]->pokemons[i]->accessor->Encrypt();
    }

    for (u32 i = 0; i < config.pokemon_teams[1]->count; i++) {
      config.pokemon_teams[1]->pokemons[i]->UpdateRuntimeData();
    }
  }
};
} // namespace feature