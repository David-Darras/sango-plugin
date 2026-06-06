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
#include "game/global_data/evolve.h"
#include "game/global_data/move.h"
#include "game/global_data/pokemon.h"

#define ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE (0x003B0C50)

namespace feature {
struct Nuzlocke {
  MAKE_SINGLETON(Nuzlocke)
  STATIC_INLINE void Initialize() {
    FixMoves();
    FixPokemon();

    HookManager::Initialize(HookID::kLoadEvolveTable,
                            ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE,
                            LoadEvolveTableHook
        );
  }

  static void LoadEvolveTableHook(uptr table, u16 species) {
    u16 current_species = READ(vu16, table + 4);
    HookManager::Call<void>(table, species);
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
};
} // namespace feature