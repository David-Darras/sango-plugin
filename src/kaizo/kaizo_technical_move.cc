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

#include "common.h"
#include "game/constant/move.h"
#include "game/constant/species.h"
#include "game/global_data/movepool.h"
#include "game/global_data/pokemon.h"
#include "game/global_data/technical_machine.h"

namespace kaizo {
static const u16 TM_MOVES[100] =
{
    MOVE_STEALTH_ROCK, MOVE_TOXIC_SPIKES, MOVE_STICKY_WEB,
    MOVE_RAPID_SPIN, MOVE_DEFOG, MOVE_TRICK_ROOM, MOVE_TAILWIND,
    MOVE_TOXIC, MOVE_WILL_O_WISP, MOVE_THUNDER_WAVE, MOVE_TAUNT,
    MOVE_ENCORE,
    MOVE_LEECH_SEED, MOVE_WHIRLWIND,
    MOVE_HAZE, MOVE_RECOVER, MOVE_ROOST, MOVE_SYNTHESIS,
    MOVE_MOONLIGHT, MOVE_SLACK_OFF, MOVE_WISH,
    MOVE_REST, MOVE_SWORDS_DANCE, MOVE_NASTY_PLOT,
    MOVE_CALM_MIND, MOVE_DRAGON_DANCE, MOVE_BULK_UP, MOVE_COIL,
    MOVE_SHELL_SMASH, MOVE_QUIVER_DANCE, MOVE_AGILITY,
    MOVE_AUTOTOMIZE, MOVE_PROTECT,
    MOVE_SUBSTITUTE, MOVE_BATON_PASS, MOVE_EXTREME_SPEED, MOVE_SUCKER_PUNCH,
    MOVE_BULLET_PUNCH, MOVE_MACH_PUNCH, MOVE_ICE_SHARD, MOVE_AQUA_JET,
    MOVE_SHADOW_SNEAK, MOVE_FAKE_OUT, MOVE_VACUUM_WAVE, MOVE_U_TURN,
    MOVE_VOLT_SWITCH, MOVE_EARTHQUAKE,
    MOVE_CLOSE_COMBAT, MOVE_SUPERPOWER, MOVE_KNOCK_OFF, MOVE_FACADE,
    MOVE_BODY_SLAM, MOVE_RETURN, MOVE_DRAIN_PUNCH, MOVE_CRUNCH,
    MOVE_NIGHT_SLASH, MOVE_IRON_HEAD, MOVE_METEOR_MASH, MOVE_STONE_EDGE,
    MOVE_ROCK_SLIDE, MOVE_ROCK_BLAST, MOVE_BRAVE_BIRD, MOVE_ACROBATICS,
    MOVE_WATERFALL, MOVE_IRON_TAIL, MOVE_POISON_JAB, MOVE_GUNK_SHOT,
    MOVE_CROSS_POISON, MOVE_SEED_BOMB, MOVE_POWER_WHIP, MOVE_WOOD_HAMMER,
    MOVE_BULLET_SEED, MOVE_FLARE_BLITZ, MOVE_OUTRAGE, MOVE_ICE_BEAM,
    MOVE_THUNDERBOLT, MOVE_FLAMETHROWER, MOVE_FIRE_BLAST, MOVE_OVERHEAT,
    MOVE_SURF, MOVE_SCALD, MOVE_HYDRO_PUMP, MOVE_DRACO_METEOR,
    MOVE_DRAGON_PULSE, MOVE_EARTH_POWER, MOVE_FLASH_CANNON, MOVE_GIGA_DRAIN,
    MOVE_HURRICANE, MOVE_PLAY_ROUGH, MOVE_MOONBLAST, MOVE_PSYCHIC,
    MOVE_DARK_PULSE
};

void PatchTechnicalMoves() {
  auto* table = global_data::TechnicalMachine::GetTable();
  for (u32 i = 0; i < SIZE(TM_MOVES); i++) {
    table[i] = TM_MOVES[i];
  }
  for (u32 species = SPECIES_BULBASAUR;
       species <= SPECIES_VOLCANION;
       species++) {
    auto& movepool = global_data::Movepool::GetInstance(species, 0);
    auto& pokemon = global_data::Pokemon::GetInstance(species, 0);
    for (u32 tm_index = 0; tm_index < SIZE(TM_MOVES); tm_index++) {
      u32* tm_bits = &pokemon.technical_moves[0];
      u32 array_index = tm_index / 32;
      u32 bit_shift = tm_index % 32;
      if (movepool.contains(TM_MOVES[tm_index])) {
        tm_bits[array_index] |= (1U << bit_shift);
      } else {
        tm_bits[array_index] &= ~(1U << bit_shift);
      }
    }
  }
}
}