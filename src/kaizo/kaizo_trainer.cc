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

#include <initializer_list>

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
#include "game/global_data/pokemon.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
static u8 s_count = 0;
static PokemonCoreData s_backup[6];

void SaveTeamBeforeBattle() {
  auto& team = savedata::PokemonTeam::GetInstance();
  s_count = team.count;
  for (u32 i = 0; i < team.count; i++) {
    auto* core = team.pokemons[i]->core;
    std::memcpy(&s_backup[i], core, sizeof(*core));
  }
}

void RestoreTeamAfterBattle() {
  if (s_count == 0) return;
  auto& team = savedata::PokemonTeam::GetInstance();
  team.count = s_count;
  for (u32 i = 0; i < team.count; i++) {
    auto* core = team.pokemons[i]->core;
    std::memcpy(core, &s_backup[i], sizeof(*core));
  }
  s_count = 0;
}

// ----------------------------------------------------------------------
// Trainer data tables.
//
// Every hand-picked trainer battle used to be a `PatchTrainer_Xxx()`
// function that imperatively called `.Set()` / `.SetStats()` /
// `.SetMoves()` on each opponent slot: ~550 lines that were really just
// data (species/item/ability/nature/EVs/moves per opponent) written as
// code. This is now a table of `TrainerSpec` values consumed by a single
// `ApplyTrainerSpec()` function, so adding/rebalancing a trainer is a
// one-line data change instead of copy-pasting a whole function.
// ----------------------------------------------------------------------

/**
* @brief One opponent slot's data: mirrors, in order, the arguments
* originally passed to PokemonCoreData::Set(), ::SetStats() and
* ::SetMoves(), plus the optional form/nickname/level overrides some
* opponents used.
*/
struct TrainerOpponentSpec {
  u16 species;
  u16 item;
  u16 ability;
  u8 nature;
  bool is_shiny;

  u8 ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense, ev_speed;

  u16 move1, move2, move3, move4;

  u8 form; // 0 = leave the species' default form untouched
  const c16* nickname; // nullptr = keep the species' default name
  u8 forced_level; // 0 = don't force a level (PatchTrainer_Level decides)

  TrainerOpponentSpec()
    : species(0), item(0), ability(0), nature(0), is_shiny(false), ev_hp(0),
      ev_attack(0), ev_defense(0), ev_sp_attack(0), ev_sp_defense(0),
      ev_speed(0), move1(0), move2(0), move3(0), move4(0), form(0),
      nickname(nullptr), forced_level(0) {
  }

  TrainerOpponentSpec(u16 species, u16 item, u16 ability, u8 nature,
                      bool is_shiny, u8 ev_hp, u8 ev_attack, u8 ev_defense,
                      u8 ev_sp_attack, u8 ev_sp_defense, u8 ev_speed,
                      u16 move1, u16 move2, u16 move3, u16 move4,
                      u8 form = 0, const c16* nickname = nullptr,
                      u8 forced_level = 0)
    : species(species), item(item), ability(ability), nature(nature),
      is_shiny(is_shiny), ev_hp(ev_hp), ev_attack(ev_attack),
      ev_defense(ev_defense), ev_sp_attack(ev_sp_attack),
      ev_sp_defense(ev_sp_defense), ev_speed(ev_speed), move1(move1),
      move2(move2), move3(move3), move4(move4), form(form),
      nickname(nickname), forced_level(forced_level) {
  }

  void ApplyTo(PokemonCoreData& pkm) const {
    pkm.Set(species, item, ability, nature, is_shiny);
    if (form != 0) pkm.form = form;
    pkm.SetStats(ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense,
                ev_speed);
    pkm.SetMoves(move1, move2, move3, move4);
    if (nickname != nullptr) pkm.SetNickname(nickname);
    if (forced_level != 0) pkm.SetLevel(forced_level);
  }
};

/**
* @brief A whole trainer battle: mirrors the arguments originally passed
* to battle::Config::Set(), plus the list of opponents and the one-off
* `config.battle_type = 0;` override that Georgia used.
*/
struct TrainerSpec {
  u8 opponent_count;
  u8 format;
  u8 background;
  u8 ground;
  u8 platform;
  u8 encounter_animation;
  u8 weather;
  bool force_wild_battle_type;
  TrainerOpponentSpec opponents[6];

  TrainerSpec(u8 opponent_count, u8 format, u8 background, u8 ground,
             u8 platform, u8 encounter_animation, u8 weather,
             std::initializer_list<TrainerOpponentSpec> opponent_list,
             bool force_wild_battle_type = false)
    : opponent_count(opponent_count), format(format), background(background),
      ground(ground), platform(platform),
      encounter_animation(encounter_animation), weather(weather),
      force_wild_battle_type(force_wild_battle_type) {
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      opponents[i++] = opponent;
    }
  }
};

void ApplyTrainerSpec(battle::Config& config, const TrainerSpec& spec) {
  if (spec.force_wild_battle_type) config.battle_type = 0;
  config.Set(spec.opponent_count, spec.format, spec.background, spec.ground,
             spec.platform, spec.encounter_animation, spec.weather);
  for (u32 i = 0; i < spec.opponent_count; i++) {
    spec.opponents[i].ApplyTo(config.GetOpponent(i));
  }
}

static const TrainerSpec kMaySpec(
    5, BATTLE_FORMAT_HORDE, BATTLE_BACKGROUND_SKY_PILLAR_TOP,
    BATTLE_GROUND_SKY_PILLAR_TOP, BATTLE_PLATFORM_SKY_PILLAR_TOP,
    BATTLE_ENCOUNTER_ANIM_RAYQUAZA, WEATHER_BATTLE_STRONG_WINDS,
    {
        {SPECIES_WHISMUR, ITEM_NONE, ABILITY_RATTLED, NATURE_MODEST, false,
         0, 0, 0, 0, 0, 0, MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE, 0,
         nullptr, 1},
        {SPECIES_WHISMUR, ITEM_LIFE_ORB, ABILITY_SOUNDPROOF, NATURE_MODEST,
         true, 244, 0, 12, 188, 0, 12, MOVE_HYPER_VOICE, MOVE_FIRE_BLAST,
         MOVE_SHADOW_BALL, MOVE_EXTRASENSORY, 0, u"Hash"},
        {SPECIES_WHISMUR, ITEM_NONE, ABILITY_RATTLED, NATURE_MODEST, false,
         0, 0, 0, 0, 0, 0, MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE, 0,
         nullptr, 1},
        {SPECIES_WHISMUR, ITEM_NONE, ABILITY_RATTLED, NATURE_MODEST, false,
         0, 0, 0, 0, 0, 0, MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE, 0,
         nullptr, 1},
        {SPECIES_WHISMUR, ITEM_NONE, ABILITY_RATTLED, NATURE_MODEST, false,
         0, 0, 0, 0, 0, 0, MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE, 0,
         nullptr, 1},
    });

static const TrainerSpec kRoute102Kid1Spec(
    2, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_AQUA_BOSS,
    BATTLE_GROUND_AQUA_BOSS, BATTLE_PLATFORM_WATER,
    BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_HEAVY_RAIN,
    {
        {SPECIES_FURFROU, ITEM_LEFTOVERS, ABILITY_FUR_COAT, NATURE_JOLLY,
         true, 0, 252, 0, 0, 4, 252, MOVE_U_TURN, MOVE_THUNDER_WAVE,
         MOVE_RETURN, MOVE_SUCKER_PUNCH, FORM_FURFROU_HEART},
        {SPECIES_POOCHYENA, ITEM_TOXIC_ORB, ABILITY_QUICK_FEET, NATURE_JOLLY,
         true, 0, 236, 0, 0, 36, 236, MOVE_CRUNCH, MOVE_PLAY_ROUGH,
         MOVE_FACADE, MOVE_FIRE_FANG},
    });

static const TrainerSpec kRoute102Kid2Spec(
    3, BATTLE_FORMAT_TRIPLE, BATTLE_BACKGROUND_MAGMA_BOSS,
    BATTLE_GROUND_MAGMA_BOSS, BATTLE_PLATFORM_VOLCANO,
    BATTLE_ENCOUNTER_ANIM_GROUDON, WEATHER_BATTLE_EXTREMELY_HARSH_SUNLIGHT,
    {
        {SPECIES_AZURILL, ITEM_EVIOLITE, ABILITY_HUGE_POWER, NATURE_ADAMANT,
         false, 116, 196, 116, 0, 36, 0, MOVE_RETURN, MOVE_KNOCK_OFF,
         MOVE_WATERFALL, MOVE_IRON_TAIL},
        {SPECIES_PICHU, ITEM_EVIOLITE, ABILITY_LIGHTNING_ROD, NATURE_TIMID,
         true, 36, 0, 0, 236, 0, 196, MOVE_NASTY_PLOT, MOVE_SUBSTITUTE,
         MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER},
        {SPECIES_RATTATA, ITEM_FLAME_ORB, ABILITY_GUTS, NATURE_JOLLY, false,
         0, 228, 76, 0, 0, 180, MOVE_FACADE, MOVE_SUCKER_PUNCH,
         MOVE_FLAME_WHEEL, MOVE_U_TURN},
    });

static const TrainerSpec kRoute102Kid3Spec(
    2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_DEOXYS, BATTLE_GROUND_DEOXYS,
    BATTLE_PLATFORM_FLYING_GYM_LEADER, BATTLE_ENCOUNTER_ANIM_DEOXYS,
    WEATHER_BATTLE_HAIL,
    {
        {SPECIES_PURRLOIN, ITEM_EVIOLITE, ABILITY_PRANKSTER, NATURE_CAREFUL,
         false, 188, 0, 180, 0, 60, 68, MOVE_ENCORE, MOVE_U_TURN,
         MOVE_KNOCK_OFF, MOVE_THUNDER_WAVE},
        {SPECIES_GLAMEOW, ITEM_LIFE_ORB, ABILITY_LIMBER, NATURE_JOLLY, false,
         0, 236, 20, 0, 0, 236, MOVE_RETURN, MOVE_KNOCK_OFF, MOVE_FAKE_OUT,
         MOVE_U_TURN},
    });

// Tiana Lass
static const TrainerSpec kRoute102GirlSpec(
    3, BATTLE_FORMAT_ROTATION, BATTLE_BACKGROUND_ABANDONED_SHIP,
    BATTLE_GROUND_ABANDONED_SHIP, BATTLE_PLATFORM_SHIP,
    BATTLE_ENCOUNTER_ANIM_HOOPA, WEATHER_BATTLE_NONE,
    {
        {SPECIES_RAIKOU, ITEM_CHOICE_SPECS, ABILITY_PRESSURE, NATURE_TIMID,
         false, 0, 0, 0, 252, 4, 252, MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT,
         MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER},
        {SPECIES_ENTEI, ITEM_CHOICE_BAND, ABILITY_PRESSURE, NATURE_ADAMANT,
         true, 0, 252, 0, 0, 4, 252, MOVE_SACRED_FIRE, MOVE_FLARE_BLITZ,
         MOVE_EXTREME_SPEED, MOVE_STONE_EDGE},
        {SPECIES_SUICUNE, ITEM_LEFTOVERS, ABILITY_PRESSURE, NATURE_TIMID,
         false, 252, 0, 0, 0, 40, 216, MOVE_SUBSTITUTE, MOVE_PROTECT,
         MOVE_CALM_MIND, MOVE_SCALD},
    });

static const TrainerSpec kRoute104YoungsterBillySpec(
    2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_SKY_BATTLE,
    BATTLE_GROUND_SKY_BATTLE, BATTLE_PLATFORM_SKY,
    BATTLE_ENCOUNTER_ANIM_GYM_LEADER_FLYING, WEATHER_BATTLE_INVALID,
    {
        {SPECIES_XATU, ITEM_ROCKY_HELMET, ABILITY_MAGIC_BOUNCE, NATURE_TIMID,
         true, 252, 0, 240, 0, 0, 16, MOVE_PSYCHIC, MOVE_ROOST,
         MOVE_GRASS_KNOT, MOVE_U_TURN},
        {SPECIES_TAILLOW, ITEM_LIFE_ORB, ABILITY_SCRAPPY, NATURE_NAIVE,
         false, 0, 0, 36, 196, 36, 236, MOVE_BOOMBURST, MOVE_HEAT_WAVE,
         MOVE_HIDDEN_POWER, MOVE_BRAVE_BIRD},
    });

static const TrainerSpec kRoute104RichBoyWinstonSpec(
    2, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_SKY_BATTLE,
    BATTLE_GROUND_SKY_BATTLE, BATTLE_PLATFORM_SKY,
    BATTLE_ENCOUNTER_ANIM_GYM_LEADER_FLYING, WEATHER_BATTLE_INVALID,
    {
        {SPECIES_PIDOVE, ITEM_LIFE_ORB, ABILITY_SUPER_LUCK, NATURE_NAIVE,
         false, 0, 236, 0, 0, 0, 252, MOVE_TAILWIND, MOVE_FACADE,
         MOVE_AERIAL_ACE, MOVE_HEAT_WAVE},
        {SPECIES_DUCKLETT, ITEM_EVIOLITE, ABILITY_HYDRATION, NATURE_CALM,
         true, 180, 0, 196, 4, 116, 0, MOVE_AIR_SLASH, MOVE_DEFOG,
         MOVE_ROOST, MOVE_SCALD},
    });

static const TrainerSpec kPetalburgWoodsBugCatcherLyleSpec(
    4, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_FOREST, BATTLE_GROUND_FOREST,
    BATTLE_PLATFORM_FOREST, BATTLE_ENCOUNTER_ANIM_RAYQUAZA,
    WEATHER_BATTLE_INVALID,
    {
        {SPECIES_METAPOD, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
         false, 248, 252, 0, 8, 0, 0, MOVE_BUG_BITE, MOVE_ELECTROWEB,
         MOVE_IRON_DEFENSE, MOVE_TACKLE},
        {SPECIES_KAKUNA, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
         true, 248, 252, 0, 8, 0, 0, MOVE_BUG_BITE, MOVE_ELECTROWEB,
         MOVE_IRON_DEFENSE, MOVE_POISON_STING},
        {SPECIES_SILCOON, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
         false, 248, 252, 0, 8, 0, 0, MOVE_ELECTROWEB, MOVE_IRON_DEFENSE,
         MOVE_BUG_BITE, MOVE_POISON_STING},
        {SPECIES_CASCOON, ITEM_EVIOLITE, ABILITY_SHED_SKIN, NATURE_NAUGHTY,
         true, 248, 252, 0, 8, 0, 0, MOVE_ELECTROWEB, MOVE_IRON_DEFENSE,
         MOVE_BUG_BITE, MOVE_POISON_STING},
    });

static const TrainerSpec kPetalburgWoodsTeamAquaGruntSpec(
    5, BATTLE_FORMAT_HORDE, BATTLE_BACKGROUND_FOREST, BATTLE_GROUND_FOREST,
    BATTLE_PLATFORM_FOREST, BATTLE_ENCOUNTER_ANIM_RAYQUAZA,
    WEATHER_BATTLE_INVALID,
    {
        {SPECIES_SCATTERBUG, ITEM_EVIOLITE, ABILITY_COMPOUND_EYES,
         NATURE_CAREFUL, true, 132, 76, 116, 0, 156, 0, MOVE_STUN_SPORE,
         MOVE_POISON_POWDER, MOVE_BUG_BITE, MOVE_TACKLE},
        {SPECIES_SEWADDLE, ITEM_EVIOLITE, ABILITY_CHLOROPHYLL, NATURE_BOLD,
         true, 156, 0, 196, 40, 116, 0, MOVE_STICKY_WEB, MOVE_GIGA_DRAIN,
         MOVE_HIDDEN_POWER, MOVE_AIR_SLASH},
        {SPECIES_WURMPLE, ITEM_EVIOLITE, ABILITY_SHIELD_DUST, NATURE_RELAXED,
         true, 156, 76, 76, 0, 196, 0, MOVE_BUG_BITE, MOVE_ELECTROWEB,
         MOVE_POISON_STING, MOVE_STRING_SHOT},
        {SPECIES_WEEDLE, ITEM_EVIOLITE, ABILITY_SHIELD_DUST, NATURE_ADAMANT,
         true, 36, 236, 36, 36, 116, 36, MOVE_BUG_BITE, MOVE_ELECTROWEB,
         MOVE_POISON_STING, MOVE_STRING_SHOT},
        {SPECIES_CATERPIE, ITEM_EVIOLITE, ABILITY_SHIELD_DUST, NATURE_RELAXED,
         true, 156, 0, 236, 0, 116, 0, MOVE_BUG_BITE, MOVE_ELECTROWEB,
         MOVE_STRING_SHOT, MOVE_TACKLE},
    });

static const TrainerSpec kPetalburgWoodsBugCatcherJamesSpec(
    2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_FOREST, BATTLE_GROUND_FOREST,
    BATTLE_PLATFORM_FOREST, BATTLE_ENCOUNTER_ANIM_RAYQUAZA,
    WEATHER_BATTLE_INVALID,
    {
        {SPECIES_BUTTERFREE, ITEM_LIFE_ORB, ABILITY_TINTED_LENS,
         NATURE_TIMID, false, 0, 0, 4, 252, 0, 252, MOVE_SLEEP_POWDER,
         MOVE_QUIVER_DANCE, MOVE_BUG_BUZZ, MOVE_ENERGY_BALL},
        {SPECIES_BEEDRILL, ITEM_BEEDRILLITE, ABILITY_SWARM, NATURE_JOLLY,
         false, 0, 252, 4, 0, 0, 252, MOVE_U_TURN, MOVE_POISON_JAB,
         MOVE_DRILL_RUN, MOVE_KNOCK_OFF},
    });

static const TrainerSpec kRoute104LadyCindySpec(
    1, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_SHORE, BATTLE_GROUND_SHORE,
    BATTLE_PLATFORM_SHORE, BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_RAIN,
    {
        {SPECIES_TOTODILE, ITEM_EVIOLITE, ABILITY_SHEER_FORCE,
         NATURE_ADAMANT, false, 0, 236, 4, 0, 0, 252, MOVE_DRAGON_DANCE,
         MOVE_WATERFALL, MOVE_ICE_PUNCH, MOVE_SUPERPOWER},
    });

static const TrainerSpec kRoute104LassHaleySpec(
    2, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_SHORE, BATTLE_GROUND_SHORE,
    BATTLE_PLATFORM_SHORE, BATTLE_ENCOUNTER_ANIM_KYOGRE, WEATHER_BATTLE_RAIN,
    {
        {SPECIES_PIPLUP, ITEM_EVIOLITE, ABILITY_TORRENT, NATURE_BOLD, true,
         92, 0, 252, 0, 148, 0, MOVE_STEALTH_ROCK, MOVE_DEFOG, MOVE_SCALD,
         MOVE_ICE_BEAM},
        {SPECIES_OSHAWOTT, ITEM_LIFE_ORB, ABILITY_TORRENT, NATURE_RASH, true,
         0, 0, 0, 252, 0, 236, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_AIR_SLASH,
         MOVE_AQUA_JET},
    });

static const TrainerSpec kRoute104TwinsGinaAndMiaSpec(
    2, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_SHORE, BATTLE_GROUND_SHORE,
    BATTLE_PLATFORM_SHORE, BATTLE_ENCOUNTER_ANIM_KYOGRE,
    WEATHER_BATTLE_HEAVY_RAIN,
    {
        {SPECIES_BLASTOISE, ITEM_BLASTOISINITE, ABILITY_RAIN_DISH,
         NATURE_MODEST, false, 248, 0, 0, 252, 8, 0, MOVE_RAPID_SPIN,
         MOVE_WATER_PULSE, MOVE_DARK_PULSE, MOVE_AURA_SPHERE},
        {SPECIES_SWAMPERT, ITEM_SWAMPERTITE, ABILITY_DAMP, NATURE_ADAMANT,
         false, 0, 252, 0, 0, 4, 252, MOVE_WATERFALL, MOVE_EARTHQUAKE,
         MOVE_ICE_PUNCH, MOVE_POWER_UP_PUNCH},
    });

static const TrainerSpec kRoute104FishermanIvanSpec(
    5, BATTLE_FORMAT_HORDE, BATTLE_BACKGROUND_SHORE, BATTLE_GROUND_SHORE,
    BATTLE_PLATFORM_SHORE, BATTLE_ENCOUNTER_ANIM_KYOGRE,
    WEATHER_BATTLE_HEAVY_RAIN,
    {
        {SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED, NATURE_JOLLY, false,
         0, 196, 0, 0, 116, 196, MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
         MOVE_BOUNCE, 0, nullptr, 1},
        {SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED, NATURE_JOLLY, false,
         0, 196, 0, 0, 116, 196, MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
         MOVE_BOUNCE, 0, nullptr, 1},
        {SPECIES_GYARADOS, ITEM_GYARADOSITE, ABILITY_INTIMIDATE,
         NATURE_JOLLY, true, 0, 252, 4, 0, 0, 252, MOVE_DRAGON_DANCE,
         MOVE_CRUNCH, MOVE_WATERFALL, MOVE_EARTHQUAKE},
        {SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED, NATURE_JOLLY, false,
         0, 196, 0, 0, 116, 196, MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
         MOVE_BOUNCE, 0, nullptr, 1},
        {SPECIES_MAGIKARP, ITEM_NONE, ABILITY_RATTLED, NATURE_JOLLY, false,
         0, 196, 0, 0, 116, 196, MOVE_SPLASH, MOVE_TACKLE, MOVE_FLAIL,
         MOVE_BOUNCE, 0, nullptr, 1},
    });

////////////////////////////////////////////////////////////////////////////////

static const TrainerSpec kRustboroCityYoungsterJoshSpec(
    4, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
    BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
    BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM,
    {
        {SPECIES_OMANYTE, ITEM_EVIOLITE, ABILITY_SHELL_ARMOR, NATURE_MODEST,
         false, 76, 0, 0, 196, 0, 236, MOVE_SHELL_SMASH, MOVE_HYDRO_PUMP,
         MOVE_ICE_BEAM, MOVE_EARTH_POWER},
        {SPECIES_KABUTO, ITEM_EVIOLITE, ABILITY_WEAK_ARMOR, NATURE_ADAMANT,
         false, 116, 196, 196, 0, 0, 0, MOVE_RAPID_SPIN, MOVE_ROCK_SLIDE,
         MOVE_KNOCK_OFF, MOVE_WATERFALL},
        {SPECIES_LILEEP, ITEM_EVIOLITE, ABILITY_STORM_DRAIN, NATURE_CALM,
         false, 228, 0, 140, 0, 140, 0, MOVE_STEALTH_ROCK, MOVE_GIGA_DRAIN,
         MOVE_RECOVER, MOVE_ANCIENT_POWER},
        {SPECIES_ANORITH, ITEM_BERRY_JUICE, ABILITY_BATTLE_ARMOR,
         NATURE_JOLLY, false, 0, 236, 36, 0, 0, 236, MOVE_STEALTH_ROCK,
         MOVE_RAPID_SPIN, MOVE_ROCK_BLAST, MOVE_KNOCK_OFF},
    });

static const TrainerSpec kRustboroCityYoungsterTommySpec(
    4, BATTLE_FORMAT_DOUBLE, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
    BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
    BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM,
    {
        {SPECIES_CRANIDOS, ITEM_LIFE_ORB, ABILITY_SHEER_FORCE, NATURE_JOLLY,
         true, 0, 236, 36, 0, 0, 212, MOVE_ROCK_SLIDE, MOVE_SUPERPOWER,
         MOVE_ZEN_HEADBUTT, MOVE_CRUNCH},
        {SPECIES_SHIELDON, ITEM_BERRY_JUICE, ABILITY_STURDY, NATURE_IMPISH,
         true, 116, 20, 212, 0, 132, 0, MOVE_STEALTH_ROCK, MOVE_HEAVY_SLAM,
         MOVE_ROCK_BLAST, MOVE_EARTHQUAKE},
        {SPECIES_TIRTOUGA, ITEM_BERRY_JUICE, ABILITY_STURDY, NATURE_JOLLY,
         false, 0, 212, 92, 0, 0, 180, MOVE_SHELL_SMASH, MOVE_WATERFALL,
         MOVE_ROCK_SLIDE, MOVE_ZEN_HEADBUTT},
        {SPECIES_ARCHEN, ITEM_BERRY_JUICE, ABILITY_DEFEATIST, NATURE_JOLLY,
         false, 76, 20, 76, 0, 76, 196, MOVE_STEALTH_ROCK, MOVE_ROCK_SLIDE,
         MOVE_ACROBATICS, MOVE_EARTHQUAKE},
    });

// This trainer forces a wild-style battle_type (see the original
// PatchTrainer_Rustboro_City_Schoolkid_Georgia, which set
// `config.battle_type = 0;` before calling `config.Set()`).
static const TrainerSpec kRustboroCitySchoolkidGeorgiaSpec(
    1, BATTLE_FORMAT_SINGLE, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
    BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
    BATTLE_ENCOUNTER_ANIM_REGIROCK, WEATHER_BATTLE_SANDSTORM,
    {
        {SPECIES_REGIROCK, ITEM_LEFTOVERS, ABILITY_CLEAR_BODY,
         NATURE_CAREFUL, false, 252, 4, 0, 0, 252, 0, MOVE_ROCK_SLIDE,
         MOVE_TOXIC, MOVE_STEALTH_ROCK, MOVE_PROTECT},
    },
    /* force_wild_battle_type = */ true);

static const TrainerSpec kRustboroCityLeaderRoxanneSpec(
    3, BATTLE_FORMAT_ROTATION, BATTLE_BACKGROUND_ROCK_GYM_LEADER_2,
    BATTLE_GROUND_ROCK_GYM_LEADER_2, BATTLE_PLATFORM_ROCK_GYM_LEADER,
    BATTLE_ENCOUNTER_ANIM_GYM_LEADER_ROCK, WEATHER_BATTLE_SANDSTORM,
    {
        {SPECIES_AERODACTYL, ITEM_AERODACTYLITE, ABILITY_ROCK_HEAD,
         NATURE_JOLLY, true, 0, 252, 0, 0, 4, 252, MOVE_STONE_EDGE,
         MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_PURSUIT},
        {SPECIES_AERODACTYL, ITEM_AERODACTYLITE, ABILITY_ROCK_HEAD,
         NATURE_JOLLY, false, 0, 252, 0, 0, 4, 252, MOVE_STONE_EDGE,
         MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_PURSUIT},
        {SPECIES_AERODACTYL, ITEM_AERODACTYLITE, ABILITY_ROCK_HEAD,
         NATURE_JOLLY, false, 0, 252, 0, 0, 4, 252, MOVE_STONE_EDGE,
         MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_PURSUIT},
    });

struct TrainerEntry {
  u16 id;
  const TrainerSpec* spec;
};

static const TrainerEntry TRAINERS[] = {
    {BATTLE_TRAINER_ROUTE_103_MAY_1, &kMaySpec},
    {BATTLE_TRAINER_ROUTE_103_MAY_2, &kMaySpec},
    {BATTLE_TRAINER_ROUTE_103_MAY_3, &kMaySpec},
    {BATTLE_TRAINER_ROUTE_102_KID_1, &kRoute102Kid1Spec},
    {BATTLE_TRAINER_ROUTE_102_KID_2, &kRoute102Kid2Spec},
    {BATTLE_TRAINER_ROUTE_102_KID_3, &kRoute102Kid3Spec},
    {BATTLE_TRAINER_ROUTE_102_GIRL, &kRoute102GirlSpec},
    {BATTLE_TRAINER_ROUTE_104_YOUNGSTER_BILLY,
     &kRoute104YoungsterBillySpec},
    {BATTLE_TRAINER_ROUTE_104_RICH_BOY_WINSTON,
     &kRoute104RichBoyWinstonSpec},
    {BATTLE_TRAINER_PETALBURG_WOODS_BUG_CATCHER_LYLE,
     &kPetalburgWoodsBugCatcherLyleSpec},
    {BATTLE_TRAINER_PETALBURG_WOODS_TEAM_AQUA_GRUNT,
     &kPetalburgWoodsTeamAquaGruntSpec},
    {BATTLE_TRAINER_PETALBURG_WOODS_BUG_CATCHER_JAMES,
     &kPetalburgWoodsBugCatcherJamesSpec},
    {BATTLE_TRAINER_ROUTE_104_LADY_CINDY, &kRoute104LadyCindySpec},
    {BATTLE_TRAINER_ROUTE_104_LASS_HALEY, &kRoute104LassHaleySpec},
    {BATTLE_TRAINER_ROUTE_104_TWINS_GINA_AND_MIA,
     &kRoute104TwinsGinaAndMiaSpec},
    {BATTLE_TRAINER_ROUTE_104_FISHERMAN_IVAN, &kRoute104FishermanIvanSpec},

    {BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_JOSH,
     &kRustboroCityYoungsterJoshSpec},
    {BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_TOMMY,
     &kRustboroCityYoungsterTommySpec},
    {BATTLE_TRAINER_RUSTBORO_CITY_SCHOOLKID_GEORGIA,
     &kRustboroCitySchoolkidGeorgiaSpec},
    {BATTLE_TRAINER_RUSTBORO_CITY_LEADER_ROXANNE,
     &kRustboroCityLeaderRoxanneSpec},
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

u32 GetPokemonBST(PokemonCoreData* core) {
  global_data::Pokemon& data =
      global_data::Pokemon::GetInstance(core->species, core->form);
  u32 bst = data.base_hp + data.base_attack + data.base_defense +
            data.base_speed + data.base_special_attack +
            data.base_special_defense;
  return bst;
}

struct TeamStats {
  u8 max_level;
  f32 avg_level;
  u32 max_bst;
  f32 avg_bst;
};

TeamStats GetTeamStats(savedata::PokemonTeam* team) {
  TeamStats stats = {0, 0.0f, 0, 0.0f};
  u32 sum_level = 0, sum_bst = 0, count = 0;

  for (u32 i = 0; i < team->count; i++) {
    if (!team->pokemons[i]) continue;

    u8 level = team->pokemons[i]->runtime->level;
    u32 bst = GetPokemonBST(team->pokemons[i]->core);

    sum_level += level;
    sum_bst += bst;
    count++;

    if (level > stats.max_level) stats.max_level = level;
    if (bst > stats.max_bst) stats.max_bst = bst;
  }

  if (count > 0) {
    stats.avg_level = (f32)sum_level / count;
    stats.avg_bst = (f32)sum_bst / count;
  }

  return stats;
}

void PatchTrainer_Level(battle::Config& config,
                        f32 nerf_strength = 0.5f,
                        f32 team_weight = 0.5f) {
  savedata::PokemonTeam* my_team = config.pokemon_teams[0];
  savedata::PokemonTeam* enemy_team = config.pokemon_teams[1];
  if (!my_team || !enemy_team) {
    return;
  }

  TeamStats my_stats = GetTeamStats(my_team);
  u8 max_level_cap = savedata::PokemonTeam::GetInstance().GetMaxLevel();

  f32 ref_level = my_stats.avg_level +
                  (my_stats.max_level - my_stats.avg_level) * team_weight;
  f32 ref_bst = my_stats.avg_bst +
                (my_stats.max_bst - my_stats.avg_bst) * team_weight;

  for (u32 i = 0; i < enemy_team->count; i++) {
    savedata::PokemonParam* enemy = enemy_team->pokemons[i];
    if (!enemy) {
      continue;
    }
    if (enemy->core->experience != 0xFFFFFFFF) {
      continue;
    }

    u32 enemy_bst = GetPokemonBST(enemy->core);
    f32 bst_ratio = (ref_bst > 0.0f) ? (f32)enemy_bst / ref_bst : 1.0f;

    s8 rand_offset = -2 + Utils::GetRandomValue(2);
    f32 target_level = ref_level + rand_offset;

    if (bst_ratio > 1.0f) {
      f32 nerf_amount = (bst_ratio - 1.0f) * nerf_strength * target_level;
      target_level -= nerf_amount;
    }

    s32 clamped = (s32)(target_level + 0.5f);
    if (clamped < 1) clamped = 1;
    if (clamped > max_level_cap) clamped = max_level_cap;

    if (config.battle_format == BATTLE_FORMAT_HORDE) {
      clamped >>= 1;
    }

    enemy->core->SetLevel((u8)clamped);
  }
}

void PatchTrainerData(battle::Config& config, u16& trainer_id) {
  ui::LogApplication::Print(u"[%u] %ls %ls wants to battle!", trainer_id,
                            config.trainer_data[1]->name->GetBuffer(),
                            config.trainer_data[1]->title_name->GetBuffer());

  // remove items
  for (u32 i = 0; i < 4; i++) {
    config.trainer_data[1]->items[i] = ITEM_NONE;
  }

  auto& battle = feature::Battle::GetInstance();

  battle.sync_team_hp = false;
  battle.inverse_stats = false;
  battle.metronome_only = false;

  // Useful to have 6 pokemon
  for (u32 i = 1; i < 6; i++) {
    *config.pokemon_teams[1]->pokemons[i]->core = *config.pokemon_teams[1]->
        pokemons[0]->core;
    *config.pokemon_teams[1]->pokemons[i]->runtime = *config.pokemon_teams[1]
        ->pokemons[0]->runtime;
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[0]->pokemons[i]->accessor->Decrypt();
    config.pokemon_teams[1]->pokemons[i]->accessor->Decrypt();
    config.pokemon_teams[1]->pokemons[i]->core->experience = 0xFFFFFFFF;
  }

  for (u32 i = 0; i < SIZE(TRAINERS); i++) {
    if (TRAINERS[i].id == trainer_id) {
      ApplyTrainerSpec(config, *TRAINERS[i].spec);
      break;
    }
  }

  PatchTrainer_Level(config);
  PatchTrainer_AI(config);

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->core->ResetNickname();
    config.pokemon_teams[1]->pokemons[i]->accessor->Encrypt();
    config.pokemon_teams[0]->pokemons[i]->accessor->Encrypt();
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->UpdateRuntimeData();
  }

  config.pokemon_teams[1]->HealAllPokemons();

  switch (trainer_id) {
    case BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_JOSH:
      config.InverseTypes();
      break;
    case BATTLE_TRAINER_RUSTBORO_CITY_YOUNGSTER_TOMMY:
      SaveTeamBeforeBattle();
      config.InverseTeams();
      break;
    case BATTLE_TRAINER_RUSTBORO_CITY_SCHOOLKID_GEORGIA:
      SaveTeamBeforeBattle();
      battle.metronome_only = true;
      break;
    case BATTLE_TRAINER_RUSTBORO_CITY_LEADER_ROXANNE:
      battle.sync_team_hp = true;
      break;
    default:
      break;
  }
}
}
