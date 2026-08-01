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
#include "parser/pokemon_node.h"
#include "parser/pokemon_showdown_parser.h"
#include "ui/log_application.h"

namespace kaizo {
void PatchTrainer_May(battle::Config& config) {
  config.pokemon_teams[1]->count = 5;
  config.battle_format = BATTLE_FORMAT_HORDE;
  config.background = BATTLE_BACKGROUND_SKY_PILLAR_TOP;
  config.ground = BATTLE_GROUND_SKY_PILLAR_TOP;
  config.platform = BATTLE_PLATFORM_SKY_PILLAR_TOP;
  config.encounter_animation = BATTLE_ENCOUNTER_ANIM_RAYQUAZA;
  config.weather = WEATHER_BATTLE_STRONG_WINDS;
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[0]->core;
    pkm.species = SPECIES_WHISMUR;
    pkm.item = ITEM_TOXIC_ORB;
    pkm.ability = ABILITY_RATTLED;
    pkm.SetShiny(false);
    pkm.SetLevel(1);
    pkm.nature = NATURE_MODEST;
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[1]->core;
    pkm.species = SPECIES_WHISMUR;
    pkm.item = ITEM_TOXIC_ORB;
    pkm.ability = ABILITY_RATTLED;
    pkm.SetShiny(false);
    pkm.SetLevel(1);
    pkm.nature = NATURE_MODEST;
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[2]->core;
    pkm.species = SPECIES_WHISMUR;
    pkm.item = ITEM_LIFE_ORB;
    pkm.ability = ABILITY_SOUNDPROOF;
    pkm.SetShiny(true);
    pkm.nature = NATURE_MODEST;
    pkm.SetStats(244, 0, 12, 188, 0, 12);
    pkm.SetMoves(MOVE_HYPER_VOICE, MOVE_FIRE_BLAST, MOVE_SHADOW_BALL,
                 MOVE_EXTRASENSORY);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[3]->core;
    pkm.species = SPECIES_WHISMUR;
    pkm.item = ITEM_TOXIC_ORB;
    pkm.ability = ABILITY_RATTLED;
    pkm.SetShiny(false);
    pkm.SetLevel(1);
    pkm.nature = NATURE_MODEST;
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[4]->core;
    pkm.species = SPECIES_WHISMUR;
    pkm.item = ITEM_TOXIC_ORB;
    pkm.ability = ABILITY_RATTLED;
    pkm.SetShiny(false);
    pkm.SetLevel(1);
    pkm.nature = NATURE_MODEST;
    pkm.SetStats(0, 0, 0, 0, 0, 0);
    pkm.SetMoves(MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE, MOVE_TACKLE);
  }
}

void PatchTrainer_Route_102_Kid1(battle::Config& config) {
  config.pokemon_teams[1]->count = 2;
  config.battle_format = BATTLE_FORMAT_DOUBLE;
  config.background = BATTLE_BACKGROUND_AQUA_BOSS;
  config.ground = BATTLE_GROUND_AQUA_BOSS;
  config.platform = BATTLE_PLATFORM_WATER;
  config.encounter_animation = BATTLE_ENCOUNTER_ANIM_KYOGRE;
  config.weather = WEATHER_BATTLE_HEAVY_RAIN;
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[0]->core;
    pkm.species = SPECIES_FURFROU;
    pkm.form = FORM_FURFROU_HEART;
    pkm.item = ITEM_LEFTOVERS;
    pkm.ability = ABILITY_FUR_COAT;
    pkm.SetShiny(true);
    pkm.nature = NATURE_JOLLY;
    pkm.SetStats(0, 252, 0, 0, 4, 252);
    pkm.SetMoves(MOVE_U_TURN, MOVE_THUNDER_WAVE, MOVE_RETURN,
                 MOVE_SUCKER_PUNCH);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[1]->core;
    pkm.species = SPECIES_POOCHYENA;
    pkm.item = ITEM_TOXIC_ORB;
    pkm.ability = ABILITY_QUICK_FEET;
    pkm.SetShiny(true);
    pkm.nature = NATURE_JOLLY;
    pkm.SetStats(0, 236, 0, 0, 36, 236);
    pkm.SetMoves(MOVE_CRUNCH, MOVE_PLAY_ROUGH, MOVE_FACADE, MOVE_FIRE_FANG);
  }
}

void PatchTrainer_Route_102_Kid2(battle::Config& config) {
  config.pokemon_teams[1]->count = 3;
  config.battle_format = BATTLE_FORMAT_TRIPLE;
  config.background = BATTLE_BACKGROUND_MAGMA_BOSS;
  config.ground = BATTLE_GROUND_MAGMA_BOSS;
  config.platform = BATTLE_PLATFORM_VOLCANO;
  config.encounter_animation = BATTLE_ENCOUNTER_ANIM_GROUDON;
  config.weather = WEATHER_BATTLE_EXTREMELY_HARSH_SUNLIGHT;
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[0]->core;
    pkm.species = SPECIES_AZURILL;
    pkm.item = ITEM_EVIOLITE;
    pkm.ability = ABILITY_HUGE_POWER;
    pkm.SetShiny(false);
    pkm.nature = NATURE_ADAMANT;
    pkm.SetStats(116, 196, 116, 0, 36, 0);
    pkm.SetMoves(MOVE_RETURN, MOVE_KNOCK_OFF, MOVE_WATERFALL,
                 MOVE_IRON_TAIL);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[1]->core;
    pkm.species = SPECIES_PICHU;
    pkm.item = ITEM_EVIOLITE;
    pkm.ability = ABILITY_LIGHTNING_ROD;
    pkm.SetShiny(true);
    pkm.nature = NATURE_TIMID;
    pkm.SetStats(36, 0, 0, 236, 0, 196);
    pkm.SetMoves(MOVE_NASTY_PLOT, MOVE_SUBSTITUTE, MOVE_THUNDERBOLT,
                 MOVE_HIDDEN_POWER);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[2]->core;
    pkm.species = SPECIES_RATTATA;
    pkm.item = ITEM_FLAME_ORB;
    pkm.ability = ABILITY_GUTS;
    pkm.SetShiny(false);
    pkm.nature = NATURE_JOLLY;
    pkm.SetStats(0, 228, 76, 0, 0, 180);
    pkm.SetMoves(MOVE_FACADE, MOVE_SUCKER_PUNCH, MOVE_FLAME_WHEEL,
                 MOVE_U_TURN);
  }
}

void PatchTrainer_Route_102_Kid3(battle::Config& config) {
  config.pokemon_teams[1]->count = 2;
  config.battle_format = BATTLE_FORMAT_SINGLE;
  config.background = BATTLE_BACKGROUND_DEOXYS;
  config.ground = BATTLE_GROUND_DEOXYS;
  config.platform = BATTLE_PLATFORM_FLYING_GYM_LEADER;
  config.encounter_animation = BATTLE_ENCOUNTER_ANIM_DEOXYS;
  config.weather = WEATHER_BATTLE_HAIL;
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[0]->core;
    pkm.species = SPECIES_PURRLOIN;
    pkm.item = ITEM_EVIOLITE;
    pkm.ability = ABILITY_PRANKSTER;
    pkm.SetShiny(false);
    pkm.nature = NATURE_CAREFUL;
    pkm.SetStats(188, 0, 180, 0, 60, 68);
    pkm.SetMoves(MOVE_ENCORE, MOVE_U_TURN, MOVE_KNOCK_OFF,
                 MOVE_THUNDER_WAVE);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[1]->core;
    pkm.species = SPECIES_GLAMEOW;
    pkm.item = ITEM_LIFE_ORB;
    pkm.ability = ABILITY_LIMBER;
    pkm.SetShiny(false);
    pkm.nature = NATURE_JOLLY;
    pkm.SetStats(0, 236, 20, 0, 0, 236);
    pkm.SetMoves(MOVE_RETURN, MOVE_KNOCK_OFF, MOVE_FAKE_OUT,
                 MOVE_U_TURN);
  }
}

void PatchTrainer_Route_102_Girl(battle::Config& config) {
  config.pokemon_teams[1]->count = 3;
  config.battle_format = BATTLE_FORMAT_ROTATION;
  config.background = BATTLE_BACKGROUND_ABANDONED_SHIP;
  config.ground = BATTLE_GROUND_ABANDONED_SHIP;
  config.platform = BATTLE_PLATFORM_SHIP;
  config.encounter_animation = BATTLE_ENCOUNTER_ANIM_HOOPA;
  config.weather = WEATHER_BATTLE_NONE;
  u8 level = savedata::PokemonTeam::GetInstance().GetMaxLevel() >> 1;
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[0]->core;
    pkm.species = SPECIES_RAIKOU;
    pkm.item = ITEM_CHOICE_SPECS;
    pkm.ability = ABILITY_PRESSURE;
    pkm.SetLevel(level);
    pkm.SetStats(0, 0, 0, 252, 4, 252);
    pkm.nature = NATURE_TIMID;
    pkm.SetShiny(false);
    pkm.SetMoves(MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT, MOVE_SHADOW_BALL,
                 MOVE_HIDDEN_POWER);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[1]->core;
    pkm.species = SPECIES_ENTEI;
    pkm.item = ITEM_CHOICE_BAND;
    pkm.ability = ABILITY_PRESSURE;
    pkm.SetLevel(level);
    pkm.SetStats(0, 252, 0, 0, 4, 252);
    pkm.nature = NATURE_ADAMANT;
    pkm.SetShiny(true);
    pkm.SetMoves(MOVE_SACRED_FIRE, MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED,
                 MOVE_STONE_EDGE);
  }
  {
    auto& pkm = *config.pokemon_teams[1]->pokemons[2]->core;
    pkm.species = SPECIES_SUICUNE;
    pkm.item = ITEM_LEFTOVERS;
    pkm.ability = ABILITY_PRESSURE;
    pkm.SetLevel(level);
    pkm.SetStats(252, 0, 0, 0, 40, 216);
    pkm.nature = NATURE_TIMID;
    pkm.SetShiny(false);
    pkm.SetMoves(MOVE_SUBSTITUTE, MOVE_PROTECT, MOVE_CALM_MIND,
                 MOVE_SCALD);
  }
}

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

void PatchTrainer_Showdown(battle::Config& config, u16& trainer_id) {
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
}

void PatchTrainerData(battle::Config& config, u16& trainer_id) {
  ui::LogApplication::Print(u"[%u] %ls %ls wants to battle!", trainer_id,
                            config.trainer_data[1]->name->GetBuffer(),
                            config.trainer_data[1]->title_name->GetBuffer());

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

  switch (trainer_id) {
    case BATTLE_TRAINER_ROUTE_103_MAY_1:
    case BATTLE_TRAINER_ROUTE_103_MAY_2:
    case BATTLE_TRAINER_ROUTE_103_MAY_3:
      PatchTrainer_May(config);
      break;
    case BATTLE_TRAINER_ROUTE_102_KID_1:
      PatchTrainer_Route_102_Kid1(config);
      break;
    case BATTLE_TRAINER_ROUTE_102_KID_2:
      PatchTrainer_Route_102_Kid2(config);
      break;
    case BATTLE_TRAINER_ROUTE_102_KID_3:
      PatchTrainer_Route_102_Kid3(config);
      break;
    case BATTLE_TRAINER_ROUTE_102_GIRL:
      PatchTrainer_Route_102_Girl(config);
      break;
    default:
      PatchTrainer_Showdown(config, trainer_id);
      break;
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
}
}