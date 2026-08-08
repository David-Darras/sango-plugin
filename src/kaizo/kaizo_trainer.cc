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
#include "feature/feature_battle.h"
#include "game/battle/config.h"
#include "game/constant/battle_format.h"
#include "game/constant/battle_trainer.h"
#include "game/constant/move.h"
#include "game/constant/ability.h"
#include "game/constant/battle_ai.h"
#include "game/constant/battle_background.h"
#include "game/constant/battle_encounter_animation.h"
#include "game/constant/battle_ground.h"
#include "game/constant/battle_platform.h"
#include "game/constant/weather.h"
#include "game/constant/form.h"
#include "game/constant/item.h"
#include "game/constant/nature.h"
#include "game/constant/species.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
static u8 GetFixLevel(f32 percent) {
  u8 level = savedata::PokemonTeam::GetInstance().GetMaxLevel();
  return static_cast<u8>(static_cast<f32>(level) * percent);
}

void PatchTrainer_May(battle::Config& config) {
  config.Set(5, BATTLE_FORMAT_HORDE, BATTLE_BACKGROUND_SKY_PILLAR_TOP,
             BATTLE_GROUND_SKY_PILLAR_TOP, BATTLE_PLATFORM_SKY_PILLAR_TOP,
             BATTLE_ENCOUNTER_ANIM_RAYQUAZA, WEATHER_BATTLE_STRONG_WINDS);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_WHISMUR, ITEM_TOXIC_ORB, ABILITY_RATTLED, NATURE_MODEST,
            false);
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
    pkm.SetLevel(1);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_WHISMUR, ITEM_TOXIC_ORB, ABILITY_RATTLED, NATURE_MODEST,
            false);
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
    pkm.SetLevel(1);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_WHISMUR, ITEM_LIFE_ORB, ABILITY_SOUNDPROOF, NATURE_MODEST,
            true);
    pkm.SetStats(244, 0, 12, 188, 0, 12);
    pkm.SetMoves(MOVE_HYPER_VOICE, MOVE_FIRE_BLAST, MOVE_SHADOW_BALL,
                 MOVE_EXTRASENSORY);
  }
  {
    auto& pkm = config.GetOpponent(3);
    pkm.Set(SPECIES_WHISMUR, ITEM_TOXIC_ORB, ABILITY_RATTLED, NATURE_MODEST,
            false);
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
    pkm.SetLevel(1);
  }
  {
    auto& pkm = config.GetOpponent(4);
    pkm.Set(SPECIES_WHISMUR, ITEM_TOXIC_ORB, ABILITY_RATTLED, NATURE_MODEST,
            false);
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
    pkm.SetLevel(1);
  }
}

void PatchTrainer_Route_102_Kid1(battle::Config& config) {
  config.Set(2, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_AQUA_BOSS,
             BATTLE_GROUND_AQUA_BOSS, BATTLE_PLATFORM_WATER,
             BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_HEAVY_RAIN);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_FURFROU, ITEM_LEFTOVERS, ABILITY_FUR_COAT, NATURE_JOLLY,
            true);
    pkm.form = FORM_FURFROU_HEART;
    pkm.SetStats(0, 252, 0, 0, 4, 252);
    pkm.SetMoves(MOVE_U_TURN, MOVE_THUNDER_WAVE, MOVE_RETURN,
                 MOVE_SUCKER_PUNCH);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_POOCHYENA, ITEM_TOXIC_ORB, ABILITY_QUICK_FEET, NATURE_JOLLY,
            true);
    pkm.SetStats(0, 236, 0, 0, 36, 236);
    pkm.SetMoves(MOVE_CRUNCH, MOVE_PLAY_ROUGH, MOVE_FACADE, MOVE_FIRE_FANG);
  }
}

void PatchTrainer_Route_102_Kid2(battle::Config& config) {
  config.Set(3, BATTLE_FORMAT_TRIPLE, BATTLE_BACKGROUND_MAGMA_BOSS,
             BATTLE_GROUND_MAGMA_BOSS, BATTLE_PLATFORM_VOLCANO,
             BATTLE_ENCOUNTER_ANIM_GROUDON,
             WEATHER_BATTLE_EXTREMELY_HARSH_SUNLIGHT);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_AZURILL, ITEM_EVIOLITE, ABILITY_HUGE_POWER, NATURE_ADAMANT,
            false);
    pkm.SetStats(116, 196, 116, 0, 36, 0);
    pkm.SetMoves(MOVE_RETURN, MOVE_KNOCK_OFF, MOVE_WATERFALL,
                 MOVE_IRON_TAIL);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_PICHU, ITEM_EVIOLITE, ABILITY_LIGHTNING_ROD, NATURE_TIMID,
            true);
    pkm.SetStats(36, 0, 0, 236, 0, 196);
    pkm.SetMoves(MOVE_NASTY_PLOT, MOVE_SUBSTITUTE, MOVE_THUNDERBOLT,
                 MOVE_HIDDEN_POWER);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_RATTATA, ITEM_FLAME_ORB, ABILITY_GUTS, NATURE_JOLLY,
            false);
    pkm.SetStats(0, 228, 76, 0, 0, 180);
    pkm.SetMoves(MOVE_FACADE, MOVE_SUCKER_PUNCH, MOVE_FLAME_WHEEL,
                 MOVE_U_TURN);
  }
}

void PatchTrainer_Route_102_Kid3(battle::Config& config) {
  config.Set(2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_DEOXYS,
             BATTLE_GROUND_DEOXYS, BATTLE_PLATFORM_FLYING_GYM_LEADER,
             BATTLE_ENCOUNTER_ANIM_DEOXYS, WEATHER_BATTLE_HAIL);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_PURRLOIN, ITEM_EVIOLITE, ABILITY_PRANKSTER, NATURE_CAREFUL,
            false);
    pkm.SetStats(188, 0, 180, 0, 60, 68);
    pkm.SetMoves(MOVE_ENCORE, MOVE_U_TURN, MOVE_KNOCK_OFF,
                 MOVE_THUNDER_WAVE);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_GLAMEOW, ITEM_LIFE_ORB, ABILITY_LIMBER, NATURE_JOLLY,
            false);
    pkm.SetStats(0, 236, 20, 0, 0, 236);
    pkm.SetMoves(MOVE_RETURN, MOVE_KNOCK_OFF, MOVE_FAKE_OUT,
                 MOVE_U_TURN);
  }
}

// Tiana Lass
void PatchTrainer_Route_102_Girl(battle::Config& config) {
  config.Set(3, BATTLE_FORMAT_ROTATION, BATTLE_BACKGROUND_ABANDONED_SHIP,
             BATTLE_GROUND_ABANDONED_SHIP, BATTLE_PLATFORM_SHIP,
             BATTLE_ENCOUNTER_ANIM_HOOPA, WEATHER_BATTLE_NONE);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_RAIKOU, ITEM_CHOICE_SPECS, ABILITY_PRESSURE, NATURE_TIMID,
            false);
    pkm.SetLevel(GetFixLevel(0.6f));
    pkm.SetStats(0, 0, 0, 252, 4, 252);
    pkm.SetMoves(MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT, MOVE_SHADOW_BALL,
                 MOVE_HIDDEN_POWER);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_ENTEI, ITEM_CHOICE_BAND, ABILITY_PRESSURE, NATURE_ADAMANT,
            true);
    pkm.SetLevel(GetFixLevel(0.6f));
    pkm.SetStats(0, 252, 0, 0, 4, 252);
    pkm.SetMoves(MOVE_SACRED_FIRE, MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED,
                 MOVE_STONE_EDGE);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_SUICUNE, ITEM_LEFTOVERS, ABILITY_PRESSURE, NATURE_TIMID,
            false);
    pkm.SetLevel(GetFixLevel(0.6f));
    pkm.SetStats(252, 0, 0, 0, 40, 216);
    pkm.SetMoves(MOVE_SUBSTITUTE, MOVE_PROTECT, MOVE_CALM_MIND,
                 MOVE_SCALD);
  }
}

void PatchTrainer_Route_104_Youngster_Billy(battle::Config& config) {
  config.Set(2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_SKY_BATTLE,
             BATTLE_GROUND_SKY_BATTLE, BATTLE_PLATFORM_SKY,
             BATTLE_ENCOUNTER_ANIM_GYM_LEADER_FLYING, WEATHER_BATTLE_INVALID);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_XATU, ITEM_ROCKY_HELMET, ABILITY_MAGIC_BOUNCE, NATURE_TIMID,
            true);
    pkm.SetStats(252, 0, 240, 0, 0, 16);
    pkm.SetMoves(MOVE_PSYCHIC, MOVE_ROOST, MOVE_GRASS_KNOT,
                 MOVE_U_TURN);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_TAILLOW, ITEM_LIFE_ORB, ABILITY_SCRAPPY, NATURE_NAIVE,
            false);
    pkm.SetStats(0, 0, 36, 196, 36, 236);
    pkm.SetMoves(MOVE_BOOMBURST, MOVE_HEAT_WAVE, MOVE_HIDDEN_POWER,
                 MOVE_BRAVE_BIRD);
  }
}

void PatchTrainer_Route_104_Rich_Boy_Winston(battle::Config& config) {
  config.Set(2, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_SKY_BATTLE,
             BATTLE_GROUND_SKY_BATTLE, BATTLE_PLATFORM_SKY,
             BATTLE_ENCOUNTER_ANIM_GYM_LEADER_FLYING, WEATHER_BATTLE_INVALID);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_PIDOVE, ITEM_LIFE_ORB, ABILITY_SUPER_LUCK, NATURE_NAIVE,
            false);
    pkm.SetStats(0, 236, 0, 0, 0, 252);
    pkm.SetMoves(MOVE_TAILWIND, MOVE_FACADE, MOVE_AERIAL_ACE,
                 MOVE_HEAT_WAVE);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_DUCKLETT, ITEM_EVIOLITE, ABILITY_HYDRATION, NATURE_CALM,
            true);
    pkm.SetStats(180, 0, 196, 4, 116, 0);
    pkm.SetMoves(MOVE_AIR_SLASH, MOVE_DEFOG, MOVE_ROOST,
                 MOVE_SCALD);
  }
}

void PatchTrainer_Petalburg_Woods_Bug_Catcher_Lyle(battle::Config& config) {
  config.Set(4, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_FOREST,
             BATTLE_GROUND_FOREST, BATTLE_PLATFORM_FOREST,
             BATTLE_ENCOUNTER_ANIM_RAYQUAZA, WEATHER_BATTLE_INVALID);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_METAPOD, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
            false);
    pkm.SetStats(248, 252, 0, 8, 0, 0);
    pkm.SetMoves(MOVE_BUG_BITE, MOVE_ELECTROWEB, MOVE_IRON_DEFENSE,
                 MOVE_TACKLE);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_KAKUNA, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
            true);
    pkm.SetStats(248, 252, 0, 8, 0, 0);
    pkm.SetMoves(MOVE_BUG_BITE, MOVE_ELECTROWEB, MOVE_IRON_DEFENSE,
                 MOVE_POISON_STING);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_SILCOON, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
            false);
    pkm.SetStats(248, 252, 0, 8, 0, 0);
    pkm.SetMoves(MOVE_ELECTROWEB, MOVE_IRON_DEFENSE, MOVE_BUG_BITE,
                 MOVE_POISON_STING);
  }
  {
    auto& pkm = config.GetOpponent(3);
    pkm.Set(SPECIES_CASCOON, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
            true);
    pkm.SetStats(248, 252, 0, 8, 0, 0);
    pkm.SetMoves(MOVE_ELECTROWEB, MOVE_IRON_DEFENSE, MOVE_BUG_BITE,
                 MOVE_POISON_STING);
  }
}

void PatchTrainer_Petalburg_Woods_Team_Aqua_Grunt(battle::Config& config) {
  u8 level = GetFixLevel(0.5f);
  config.Set(5, BATTLE_FORMAT_HORDE, BATTLE_BACKGROUND_FOREST,
             BATTLE_GROUND_FOREST, BATTLE_PLATFORM_FOREST,
             BATTLE_ENCOUNTER_ANIM_RAYQUAZA, WEATHER_BATTLE_INVALID);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_SCATTERBUG, ITEM_EVIOLITE, ABILITY_COMPOUND_EYES,
            NATURE_CAREFUL,
            true);
    pkm.SetStats(132, 76, 116, 0, 156, 0);
    pkm.SetMoves(MOVE_STUN_SPORE, MOVE_POISON_POWDER, MOVE_BUG_BITE,
                 MOVE_TACKLE);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_SEWADDLE, ITEM_EVIOLITE, ABILITY_CHLOROPHYLL, NATURE_BOLD,
            true);
    pkm.SetStats(156, 0, 196, 40, 116, 0);
    pkm.SetMoves(MOVE_STICKY_WEB, MOVE_GIGA_DRAIN, MOVE_HIDDEN_POWER,
                 MOVE_AIR_SLASH);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_WURMPLE, ITEM_EVIOLITE, ABILITY_SHIELD_DUST, NATURE_RELAXED,
            true);
    pkm.SetStats(156, 76, 76, 0, 196, 0);
    pkm.SetMoves(MOVE_BUG_BITE, MOVE_ELECTROWEB, MOVE_POISON_STING,
                 MOVE_STRING_SHOT);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(3);
    pkm.Set(SPECIES_WEEDLE, ITEM_EVIOLITE, ABILITY_SHIELD_DUST, NATURE_ADAMANT,
            true);
    pkm.SetStats(36, 236, 36, 36, 116, 36);
    pkm.SetMoves(MOVE_BUG_BITE, MOVE_ELECTROWEB, MOVE_POISON_STING,
                 MOVE_STRING_SHOT);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(4);
    pkm.Set(SPECIES_CATERPIE, ITEM_EVIOLITE, ABILITY_SHIELD_DUST,
            NATURE_RELAXED,
            true);
    pkm.SetStats(156, 0, 236, 0, 116, 0);
    pkm.SetMoves(MOVE_BUG_BITE, MOVE_ELECTROWEB, MOVE_STRING_SHOT,
                 MOVE_TACKLE);
    pkm.SetLevel(level);
  }
}

void PatchTrainer_Petalburg_Woods_Bug_Catcher_James(battle::Config& config) {
  u8 level = GetFixLevel(0.7f);
  config.Set(2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_FOREST,
             BATTLE_GROUND_FOREST, BATTLE_PLATFORM_FOREST,
             BATTLE_ENCOUNTER_ANIM_RAYQUAZA, WEATHER_BATTLE_INVALID);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_BUTTERFREE, ITEM_LIFE_ORB, ABILITY_TINTED_LENS,
            NATURE_TIMID,
            false);
    pkm.SetStats(0, 0, 4, 252, 0, 252);
    pkm.SetMoves(MOVE_SLEEP_POWDER, MOVE_QUIVER_DANCE, MOVE_BUG_BUZZ,
                 MOVE_ENERGY_BALL);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_BEEDRILL, ITEM_BEEDRILLITE, ABILITY_SWARM, NATURE_JOLLY,
            false);
    pkm.SetStats(0, 252, 4, 0, 0, 252);
    pkm.SetMoves(MOVE_U_TURN, MOVE_POISON_JAB, MOVE_DRILL_RUN,
                 MOVE_KNOCK_OFF);
    pkm.SetLevel(level);
  }
}

void PatchTrainer_Route_104_Lady_Cindy(battle::Config& config) {
  u8 level = GetFixLevel(0.8f);
  config.Set(1, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_SHORE,
             BATTLE_GROUND_SHORE, BATTLE_PLATFORM_SHORE,
             BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_RAIN);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_TOTODILE, ITEM_EVIOLITE, ABILITY_SHEER_FORCE,
            NATURE_ADAMANT,
            false);
    pkm.SetStats(0, 236, 4, 0, 0, 252);
    pkm.SetMoves(MOVE_DRAGON_DANCE, MOVE_WATERFALL, MOVE_ICE_PUNCH,
                 MOVE_SUPERPOWER);
    pkm.SetLevel(level);
  }
}

void PatchTrainer_Route_104_Lass_Haley(battle::Config& config) {
  u8 level = GetFixLevel(0.8f);
  config.Set(2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_SHORE,
             BATTLE_GROUND_SHORE, BATTLE_PLATFORM_SHORE,
             BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_RAIN);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_PIPLUP, ITEM_EVIOLITE, ABILITY_TORRENT,
            NATURE_BOLD,
            true);
    pkm.SetStats(92, 0, 252, 0, 148, 0);
    pkm.SetMoves(MOVE_STEALTH_ROCK, MOVE_DEFOG, MOVE_SCALD,
                 MOVE_ICE_BEAM);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_OSHAWOTT, ITEM_LIFE_ORB, ABILITY_TORRENT,
            NATURE_RASH,
            true);
    pkm.SetStats(0, 0, 0, 252, 0, 236);
    pkm.SetMoves(MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_AIR_SLASH,
                 MOVE_AQUA_JET);
    pkm.SetLevel(level);
  }
}

void PatchTrainer_Route_104_Twins_Gina_and_Mia(battle::Config& config) {
  u8 level = GetFixLevel(0.7f);
  config.Set(2, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_SHORE,
             BATTLE_GROUND_SHORE, BATTLE_PLATFORM_SHORE,
             BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_HEAVY_RAIN);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_BLASTOISE, ITEM_BLASTOISINITE, ABILITY_RAIN_DISH,
            NATURE_MODEST,
            false);
    pkm.SetStats(248, 0, 0, 252, 8, 0);
    pkm.SetMoves(MOVE_RAPID_SPIN, MOVE_WATER_PULSE, MOVE_DARK_PULSE,
                 MOVE_AURA_SPHERE);
    pkm.SetLevel(level);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_SWAMPERT, ITEM_SWAMPERTITE, ABILITY_DAMP,
            NATURE_ADAMANT,
            false);
    pkm.SetStats(0, 252, 0, 0, 4, 252);
    pkm.SetMoves(MOVE_WATERFALL, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH,
                 MOVE_POWER_UP_PUNCH);
    pkm.SetLevel(level);
  }
}

void PatchTrainer_Route_104_Fisherman_Ivan(battle::Config& config) {
  config.Set(5, BATTLE_FORMAT_HORDE, BATTLE_BACKGROUND_SHORE,
             BATTLE_GROUND_SHORE, BATTLE_PLATFORM_SHORE,
             BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_HEAVY_RAIN);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 196, 0, 0, 116, 196);
    pkm.SetMoves(MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
                 MOVE_BOUNCE);
    pkm.SetLevel(1);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 196, 0, 0, 116, 196);
    pkm.SetMoves(MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
                 MOVE_BOUNCE);
    pkm.SetLevel(1);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_GYARADOS, ITEM_GYARADOSITE, ABILITY_INTIMIDATE,
            NATURE_JOLLY,
            true);
    pkm.SetStats(0, 252, 4, 0, 0, 252);
    pkm.SetMoves(MOVE_DRAGON_DANCE, MOVE_CRUNCH, MOVE_WATERFALL,
                 MOVE_EARTHQUAKE);
    pkm.SetLevel(GetFixLevel(0.6f));
  }
  {
    auto& pkm = config.GetOpponent(3);
    pkm.Set(SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 196, 0, 0, 116, 196);
    pkm.SetMoves(MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
                 MOVE_BOUNCE);
    pkm.SetLevel(1);
  }
  {
    auto& pkm = config.GetOpponent(4);
    pkm.Set(SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 196, 0, 0, 116, 196);
    pkm.SetMoves(MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
                 MOVE_BOUNCE);
    pkm.SetLevel(1);
  }
}

////////////////////////////////////////////////////////////////////////////////

void PatchTrainer_Rustboro_City_Youngster_Josh(battle::Config& config) {
  config.Set(4, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
             BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
             BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_OMANYTE, ITEM_EVIOLITE, ABILITY_SHELL_ARMOR,
            NATURE_MODEST,
            false);
    pkm.SetStats(76, 0, 0, 196, 0, 236);
    pkm.SetMoves(MOVE_SHELL_SMASH, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM,
                 MOVE_EARTH_POWER);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_KABUTO, ITEM_EVIOLITE, ABILITY_WEAK_ARMOR,
            NATURE_ADAMANT,
            false);
    pkm.SetStats(116, 196, 196, 0, 0, 0);
    pkm.SetMoves(MOVE_RAPID_SPIN, MOVE_ROCK_SLIDE, MOVE_KNOCK_OFF,
                 MOVE_WATERFALL);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_LILEEP, ITEM_EVIOLITE, ABILITY_STORM_DRAIN,
            NATURE_CALM,
            false);
    pkm.SetStats(228, 0, 140, 0, 140, 0);
    pkm.SetMoves(MOVE_STEALTH_ROCK, MOVE_GIGA_DRAIN, MOVE_RECOVER,
                 MOVE_ANCIENT_POWER);
  }
  {
    auto& pkm = config.GetOpponent(3);
    pkm.Set(SPECIES_ANORITH, ITEM_BERRY_JUICE, ABILITY_BATTLE_ARMOR,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 236, 36, 0, 0, 236);
    pkm.SetMoves(MOVE_STEALTH_ROCK, MOVE_RAPID_SPIN, MOVE_ROCK_BLAST,
                 MOVE_KNOCK_OFF);
  }
}

void PatchTrainer_Rustboro_City_Youngster_Tommy(battle::Config& config) {
  config.Set(4, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
             BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
             BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_CRANIDOS, ITEM_LIFE_ORB, ABILITY_SHEER_FORCE,
            NATURE_JOLLY,
            true);
    pkm.SetStats(0, 236, 36, 0, 0, 212);
    pkm.SetMoves(MOVE_ROCK_SLIDE, MOVE_SUPERPOWER, MOVE_ZEN_HEADBUTT,
                 MOVE_CRUNCH);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_SHIELDON, ITEM_BERRY_JUICE, ABILITY_STURDY,
            NATURE_IMPISH,
            true);
    pkm.SetStats(116, 20, 212, 0, 132, 0);
    pkm.SetMoves(MOVE_STEALTH_ROCK, MOVE_HEAVY_SLAM, MOVE_ROCK_BLAST,
                 MOVE_EARTHQUAKE);
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_TIRTOUGA, ITEM_BERRY_JUICE, ABILITY_STURDY,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 212, 92, 0, 0, 180);
    pkm.SetMoves(MOVE_SHELL_SMASH, MOVE_WATERFALL, MOVE_ROCK_SLIDE,
                 MOVE_ZEN_HEADBUTT);
  }
  {
    auto& pkm = config.GetOpponent(3);
    pkm.Set(SPECIES_ARCHEN, ITEM_BERRY_JUICE, ABILITY_DEFEATIST,
            NATURE_JOLLY,
            false);
    pkm.SetStats(76, 20, 76, 0, 76, 196);
    pkm.SetMoves(MOVE_STEALTH_ROCK, MOVE_ROCK_SLIDE, MOVE_ACROBATICS,
                 MOVE_EARTHQUAKE);
  }
}

void PatchTrainer_Rustboro_City_Schoolkid_Georgia(battle::Config& config) {
  config.Set(3, BATTLE_FORMAT_ROTATION, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
             BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
             BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_TYRUNT, ITEM_BERRY_JUICE, ABILITY_STURDY,
            NATURE_JOLLY,
            false);
    pkm.SetStats(0, 204, 0, 0, 76, 212);
    pkm.SetMoves(MOVE_DRAGON_DANCE, MOVE_STONE_EDGE, MOVE_OUTRAGE,
                 MOVE_EARTHQUAKE);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_REGIROCK, ITEM_LEFTOVERS, ABILITY_CLEAR_BODY,
            NATURE_CAREFUL,
            false);
    pkm.SetStats(252, 4, 0, 0, 252, 0);
    pkm.SetMoves(MOVE_ROCK_SLIDE, MOVE_TOXIC, MOVE_STEALTH_ROCK,
                 MOVE_PROTECT);
    pkm.SetLevel(GetFixLevel(0.7f));
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_AMAURA, ITEM_CHOICE_SCARF, ABILITY_REFRIGERATE,
            NATURE_MODEST,
            false);
    pkm.SetStats(60, 0, 0, 220, 0, 228);
    pkm.SetMoves(MOVE_HYPER_VOICE, MOVE_EARTH_POWER, MOVE_THUNDERBOLT,
                 MOVE_HYPER_BEAM);
  }
}

void PatchTrainer_Rustboro_City_Leader_Roxanne(battle::Config& config) {
  config.Set(3, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
             BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
             BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM);
  {
    auto& pkm = config.GetOpponent(0);
    pkm.Set(SPECIES_GEODUDE, ITEM_BERRY_JUICE, ABILITY_STURDY,
            NATURE_ADAMANT,
            false);
    pkm.SetStats(36, 196, 196, 0, 36, 36);
    pkm.SetMoves(MOVE_STEALTH_ROCK, MOVE_EXPLOSION, MOVE_EARTHQUAKE,
                 MOVE_STONE_EDGE);
  }
  {
    auto& pkm = config.GetOpponent(1);
    pkm.Set(SPECIES_CARBINK, ITEM_LEFTOVERS, ABILITY_CLEAR_BODY,
            NATURE_BOLD,
            false);
    pkm.SetStats(248, 0, 252, 8, 0, 0);
    pkm.SetMoves(MOVE_CALM_MIND, MOVE_MOONBLAST, MOVE_REST,
                 MOVE_SLEEP_TALK);
    pkm.SetLevel(GetFixLevel(0.7f));
  }
  {
    auto& pkm = config.GetOpponent(2);
    pkm.Set(SPECIES_AERODACTYL, ITEM_AERODACTYLITE, ABILITY_ROCK_HEAD,
            NATURE_JOLLY,
            true);
    pkm.SetStats(0, 252, 0, 0, 4, 252);
    pkm.SetMoves(MOVE_STONE_EDGE, MOVE_AERIAL_ACE, MOVE_EARTHQUAKE,
                 MOVE_PURSUIT);
    pkm.SetLevel(GetFixLevel(0.85f));
  }
}

struct NewTrainer {
  u16 id;
  void (*patch)(battle::Config& config);
};

static NewTrainer TRAINERS[] = {
    {BATTLE_TRAINER_ROUTE_103_MAY_1, PatchTrainer_May},
    {BATTLE_TRAINER_ROUTE_103_MAY_2, PatchTrainer_May},
    {BATTLE_TRAINER_ROUTE_103_MAY_3, PatchTrainer_May},
    {BATTLE_TRAINER_ROUTE_102_KID_1, PatchTrainer_Route_102_Kid1},
    {BATTLE_TRAINER_ROUTE_102_KID_2, PatchTrainer_Route_102_Kid2},
    {BATTLE_TRAINER_ROUTE_102_KID_3, PatchTrainer_Route_102_Kid3},
    {BATTLE_TRAINER_ROUTE_102_GIRL, PatchTrainer_Route_102_Girl},
    {BATTLE_TRAINER_ROUTE_104_YOUNGSTER_BILLY, PatchTrainer_Route_104_Youngster_Billy},
    {BATTLE_TRAINER_ROUTE_104_RICH_BOY_WINSTON, PatchTrainer_Route_104_Rich_Boy_Winston},
    {BATTLE_TRAINER_PETALBURG_WOODS_BUG_CATCHER_LYLE, PatchTrainer_Petalburg_Woods_Bug_Catcher_Lyle},
    {BATTLE_TRAINER_PETALBURG_WOODS_TEAM_AQUA_GRUNT, PatchTrainer_Petalburg_Woods_Team_Aqua_Grunt},
    {BATTLE_TRAINER_PETALBURG_WOODS_BUG_CATCHER_JAMES, PatchTrainer_Petalburg_Woods_Bug_Catcher_James},
    {BATTLE_TRAINER_ROUTE_104_LADY_CINDY, PatchTrainer_Route_104_Lady_Cindy},
    {BATTLE_TRAINER_ROUTE_104_LASS_HALEY, PatchTrainer_Route_104_Lass_Haley},
    {BATTLE_TRAINER_ROUTE_104_TWINS_GINA_AND_MIA, PatchTrainer_Route_104_Twins_Gina_and_Mia},
    {BATTLE_TRAINER_ROUTE_104_FISHERMAN_IVAN, PatchTrainer_Route_104_Fisherman_Ivan},

    {BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_JOSH, PatchTrainer_Rustboro_City_Youngster_Josh},
    {BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_TOMMY, PatchTrainer_Rustboro_City_Youngster_Tommy},
    {BATTLE_TRAINER_RUSTBORO_CITY_SCHOOLKID_GEORGIA, PatchTrainer_Rustboro_City_Schoolkid_Georgia},
    {BATTLE_TRAINER_RUSTBORO_CITY_LEADER_ROXANNE, PatchTrainer_Rustboro_City_Leader_Roxanne},
};

void PatchTrainer_AI(battle::Config& config) {
  // Best AI
  u32 flags;
  switch (config.battle_format) {
    case BATTLE_FORMAT_DOUBLE:
    case BATTLE_FORMAT_TRIPLE:
    case BATTLE_FORMAT_ROTATION:
      flags = BATTLE_AI_CASUAL | BATTLE_AI_COMPETITIVE | BATTLE_AI_STRATEGIST |
              BATTLE_AI_MULTI;
      break;
    case BATTLE_FORMAT_HORDE:
      flags = BATTLE_AI_CASUAL | BATTLE_AI_COMPETITIVE | BATTLE_AI_STRATEGIST |
              BATTLE_AI_HORDE;
      break;
    case BATTLE_FORMAT_SINGLE:
    default:
      flags = BATTLE_AI_CASUAL | BATTLE_AI_COMPETITIVE | BATTLE_AI_STRATEGIST;
      break;
  }

  if (config.trainer_data[0] != nullptr)
    config.trainer_data[0]->ai_flags = flags;
  if (config.trainer_data[1] != nullptr)
    config.trainer_data[1]->ai_flags = flags;
  if (config.trainer_data[2] != nullptr)
    config.trainer_data[2]->ai_flags = flags;
  if (config.trainer_data[3] != nullptr)
    config.trainer_data[3]->ai_flags = flags;
}

void PatchTrainer_Level(battle::Config& config) {
  for (u32 i = 0; i < config.pokemon_teams[1]->count; i++) {
    u8 max = savedata::PokemonTeam::GetInstance().GetMaxLevel();
    s8 rand = -2 + Utils::GetRandomValue(2);
    if (config.pokemon_teams[1]->pokemons[i]->core->experience == 0xFFFFFFFF) {
      config.pokemon_teams[1]->pokemons[i]->core->SetLevel(rand + max);
    }
  }
}

void PatchTrainerData(battle::Config& config, u16& trainer_id) {
  ui::LogApplication::Print(u"[%u] %ls %ls wants to battle!", trainer_id,
                            config.trainer_data[1]->name->GetBuffer(),
                            config.trainer_data[1]->title_name->GetBuffer());

  feature::Battle::GetInstance().sync_team_hp = false;

  // Useful to have 6 pokemon
  for (u32 i = 1; i < 6; i++) {
    *config.pokemon_teams[1]->pokemons[i]->core = *config.pokemon_teams[1]->
        pokemons[0]->core;
    *config.pokemon_teams[1]->pokemons[i]->runtime = *config.pokemon_teams[1]
        ->pokemons[0]->runtime;
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->accessor->Decrypt();
    config.pokemon_teams[1]->pokemons[i]->core->experience = 0xFFFFFFFF;
  }

  for (u32 i = 0; i < SIZE(TRAINERS); i++) {
    if (TRAINERS[i].id == trainer_id) {
      TRAINERS[i].patch(config);
      break;
    }
  }

  PatchTrainer_Level(config);
  PatchTrainer_AI(config);

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->core->ResetNickname();
    config.pokemon_teams[1]->pokemons[i]->accessor->Encrypt();
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->UpdateRuntimeData();
  }

  config.pokemon_teams[1]->HealAllPokemons();

  switch (trainer_id) {
    case BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_JOSH:
      feature::Battle::GetInstance().sync_team_hp = true;
      break;
    case BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_TOMMY:
      config.Inverse();
      break;
  }
}
}