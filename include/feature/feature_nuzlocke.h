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
#include "archive.h"
#include "common.h"
#include "feature_camera.h"
#include "feature_light.h"
#include "hook_manager.h"
#include "utils.h"
#include "game/battle/config.h"
#include "game/constant/evolution_method.h"
#include "game/constant/item.h"
#include "game/constant/model.h"
#include "game/constant/species.h"
#include "game/global_data/evolve.h"
#include "game/global_data/move.h"
#include "game/global_data/pokemon.h"
#include "game/global_data/trainer_model_manager.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/settings.h"
#include "game/savedata/trainer_status.h"
#include "parser/pokemon_showdown_parser.h"
#include "parser/pokemon_node.h"
#include "ui/log_application.h"

namespace overworld {
struct EncounterData;
}

struct EncounterEntry {
  const u16 map_id;
  const u16 size;
  const u16* species;
};

namespace feature {
struct Nuzlocke {
  MAKE_SINGLETON(Nuzlocke)
  uptr archive_buffer = 0;
  u32* size = nullptr;
  u32 trainer_id = 0;

  STATIC_INLINE void Initialize() {
    FixMoves();
    FixPokemon();
    FixOutline();
    FixTrainerModels();

    HookManager::Initialize(HookID::kLoadEvolveTable,
                            ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE,
                            (uptr)LoadEvolveTableHook
        );
    HookManager::Initialize(HookID::kAddPokemonToTeam,
                            ADDRESS_ADD_POKEMON_TO_TEAM,
                            (uptr)AddPokemonToTeamHook);
    HookManager::Initialize(HookID::kCreateOverworldModels, 0x003F8358, (uptr)
                            CreateOverworldModelsHook);
    HookManager::Initialize(HookID::kReadFileAsync, 0x0036DC68,
                            (uptr)ReadFileAsync);
    HookManager::Initialize(HookID::kReadFileAsync2, 0x0036ED10,
                            (uptr)ReadFileAsync2);
    HookManager::Initialize(HookID::kIsReadFinished, 0x004C7224,
                            (uptr)IsReadFinished);
    HookManager::Initialize(HookID::kInitializePokemon, 0x0011F754,
                            (uptr)InitializePokemonHook);
    HookManager::Initialize(HookID::kIsShiny, 0x00168F48,
                            (uptr)IsShinyHook);
    HookManager::Initialize(HookID::kFromNormalToShiny,
                            ADDRESS_POKEMON_UTILS_TO_SHINY,
                            (uptr)ToShinyHook);
    HookManager::Initialize(HookID::kFromShinyToNormal,
                            ADDRESS_POKEMON_UTILS_TO_NORMAL,
                            (uptr)ToNormalHook);
  }

  STATIC_INLINE void FixTrainerModels() {
    auto& manager = TrainerModelManager::GetInstance();
    manager.Replace(TRAINER_MODEL_BRENDAN, TRAINER_MODEL_STEVEN);
    manager.Replace(TRAINER_MODEL_MAY, TRAINER_MODEL_ZINNIA);
  }
  static u32 ToShinyHook(u32 id, u32 pid) {
    return 0b010;
  }

  static u32 ToNormalHook(u32 id, u32 pid) {
    return 0b101;
  }

  // probability : 1/8
  static bool IsShinyHook(u32 id, u32 pid) {
    return (pid & 0b111) == 0b010;
  }

  static u32 InitializePokemonHook(savedata::PokemonParam* param, u32 heap,
                                   u32 init) {
    struct Pokemon {
      u64 id[3];
      u16 species;
      u8 form;
      u8 level;
    }& pokemon = *(Pokemon*)init;
    static const u16 GRASS_STARTERS[] = {
        SPECIES_BULBASAUR, SPECIES_CHIKORITA,
        SPECIES_TREECKO, SPECIES_TURTWIG,
        SPECIES_SNIVY, SPECIES_CHESPIN
    };
    u16 rand = Utils::GetRandomValue(SIZE(GRASS_STARTERS));
    switch (pokemon.species) {
      case SPECIES_TREECKO:
        pokemon.species = GRASS_STARTERS[rand] + 0;
        break;
      case SPECIES_TORCHIC:
        pokemon.species = GRASS_STARTERS[rand] + 3;
        break;
      case SPECIES_MUDKIP:
        pokemon.species = GRASS_STARTERS[rand] + 6;
        break;
    }
    return HookManager::Call<
      u32>(HookID::kInitializePokemon, param, heap, init);
  }

  struct FileInput {
    u8 priority;
    u32 archive_id;
    u32 file_id;
    bool is_compressed;
    uptr heap[4];
    uptr buffer;
    u32* size;
  };

  static bool ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                             void* buffer,
                             u32 p4, u32 p5, u32 p6) {
    u32* archive_table = (u32*)ADDRESS_ARCHIVE_FILENAME_TABLE;
    if (archive[12] == archive_table[ARCHIVE_OVERWORLD_MODEL]) {
      file_id = FixOverworldModels(file_id, true);
    }
    return HookManager::Call<bool>(HookID::kReadFileAsync2, archive, heap,
                                   file_id,
                                   buffer, p4, p5, p6);
  }

  static bool ReadFileAsync(void* file_manager, FileInput* input) {
    if (input->archive_id == ARCHIVE_OVERWORLD_MODEL) {
      input->file_id = FixOverworldModels(input->file_id, false);
    }
    if (input->archive_id == ARCHIVE_PLAYER_ICON) {
      input->file_id = 72; // STEVEN
    }
    if (input->archive_id == 262) {
      ui::LogApplication::Print(u"262");
      GetInstance().archive_buffer = input->buffer;
      GetInstance().size = input->size;
    }
    return HookManager::Call<bool>(HookID::kReadFileAsync, file_manager, input);
  }

  static bool IsReadFinished(void* file_manager, void** buffer) {
    bool result = HookManager::Call<bool>(HookID::kIsReadFinished, file_manager,
                                          buffer);
    if (result && (uptr)buffer == GetInstance().archive_buffer) {
    }
    return result;
  }

  struct ModelData {
    u16 uid;
    u16 model_id;
    u8 _0[44];
  };

  STATIC_INLINE u32 FixOverworldModels(u32 model, bool is_overworld) {
    if (!is_overworld) {
      switch (model) {
        case MODEL_ZIGZAGOON:
          return MODEL_GROUDON;
        case MODEL_SKITTY:
          return MODEL_KYOGRE;
      }
    }
    switch (model) {
      case MODEL_BRENDAN:
        return MODEL_STEVEN_STONE;
      case MODEL_SUPPORT_MAY:
        return MODEL_ZINNIA;
      case MODEL_MOM_ORAS:
        return MODEL_MR_STONE;
      case MODEL_SNORLAX_DOLL:
        return MODEL_HOOPAS_RING;
    }
    return model;
  }

  STATIC_INLINE void FixOutline() {
    auto& light = Light::GetInstance();
    light.use_outline = true;
    light.outline_scale = 0.0f;
  }

  STATIC_INLINE void FixConfig() {
    auto& data = savedata::Settings::GetInstance();
    data.text_speed = 3; // Instant message
    data.battle_style = 1;
    data.show_battle_animations = 0;
  }

  static bool CreateOverworldModelsHook(uptr man, ModelData* table, u32 max,
                                        u16 map_id,
                                        void** fashion) {
    return HookManager::Call<bool>(HookID::kCreateOverworldModels, man, table,
                                   max, map_id, fashion);
  }

  static bool AddPokemonToTeamHook(savedata::PokemonTeam* team,
                                   savedata::PokemonParam* pokemon) {
    if (team != &savedata::PokemonTeam::GetInstance()) {
      return HookManager::Call<bool>(HookID::kAddPokemonToTeam, team, pokemon);
    }

    pokemon->accessor->Decrypt();
    u16 species = pokemon->core->species;
    pokemon->accessor->Encrypt();

    static const u16 all_special_pokemon[] = {
        // --- GENERATION 1 ---
        SPECIES_ARTICUNO, // Sub-Legendary (Allowed)
        SPECIES_ZAPDOS, // Sub-Legendary (Allowed)
        SPECIES_MOLTRES, // Sub-Legendary (Allowed)
        SPECIES_MEWTWO, // Restricted Legendary (Banned)
        SPECIES_MEW, // Mythical (Banned)

        // --- GENERATION 2 ---
        SPECIES_RAIKOU, // Sub-Legendary (Allowed)
        SPECIES_ENTEI, // Sub-Legendary (Allowed)
        SPECIES_SUICUNE, // Sub-Legendary (Allowed)
        SPECIES_LUGIA, // Restricted Legendary (Banned)
        SPECIES_HO_OH, // Restricted Legendary (Banned)
        SPECIES_CELEBI, // Mythical (Banned)

        // --- GENERATION 3 ---
        SPECIES_REGIROCK, // Sub-Legendary (Allowed)
        SPECIES_REGICE, // Sub-Legendary (Allowed)
        SPECIES_REGISTEEL, // Sub-Legendary (Allowed)
        SPECIES_LATIAS, // Sub-Legendary (Allowed)
        SPECIES_LATIOS, // Sub-Legendary (Allowed)
        SPECIES_KYOGRE, // Restricted Legendary (Banned)
        SPECIES_GROUDON, // Restricted Legendary (Banned)
        SPECIES_RAYQUAZA, // Restricted Legendary (Banned)
        SPECIES_JIRACHI, // Mythical (Banned)
        SPECIES_DEOXYS, // Mythical (Banned)

        // --- GENERATION 4 ---
        SPECIES_UXIE, // Sub-Legendary (Allowed)
        SPECIES_MESPRIT, // Sub-Legendary (Allowed)
        SPECIES_AZELF, // Sub-Legendary (Allowed)
        SPECIES_DIALGA, // Restricted Legendary (Banned)
        SPECIES_PALKIA, // Restricted Legendary (Banned)
        SPECIES_HEATRAN, // Sub-Legendary (Allowed)
        SPECIES_REGIGIGAS, // Sub-Legendary (Allowed)
        SPECIES_GIRATINA, // Restricted Legendary (Banned)
        SPECIES_CRESSELIA, // Sub-Legendary (Allowed)
        SPECIES_PHIONE, // Mythical (Banned)
        SPECIES_MANAPHY, // Mythical (Banned)
        SPECIES_DARKRAI, // Mythical (Banned)
        SPECIES_SHAYMIN, // Mythical (Banned)
        SPECIES_ARCEUS, // Mythical (Banned)

        // --- GENERATION 5 ---
        SPECIES_COBALION, // Sub-Legendary (Allowed)
        SPECIES_TERRAKION, // Sub-Legendary (Allowed)
        SPECIES_VIRIZION, // Sub-Legendary (Allowed)
        SPECIES_TORNADUS, // Sub-Legendary (Allowed)
        SPECIES_THUNDURUS, // Sub-Legendary (Allowed)
        SPECIES_RESHIRAM, // Restricted Legendary (Banned)
        SPECIES_ZEKROM, // Restricted Legendary (Banned)
        SPECIES_LANDORUS, // Sub-Legendary (Allowed)
        SPECIES_KYUREM, // Restricted Legendary (Banned)
        SPECIES_VICTINI, // Mythical (Banned)
        SPECIES_KELDEO, // Mythical (Banned)
        SPECIES_MELOETTA, // Mythical (Banned)
        SPECIES_GENESECT, // Mythical (Banned)

        // --- GENERATION 6 ---
        SPECIES_XERNEAS, // Restricted Legendary (Banned)
        SPECIES_YVELTAL, // Restricted Legendary (Banned)
        SPECIES_ZYGARDE, // Restricted Legendary (Banned)
        SPECIES_DIANCIE, // Mythical (Banned)
        SPECIES_HOOPA, // Mythical (Banned)
        SPECIES_VOLCANION, // Mythical (Banned)
    };
    for (u32 i = 0; i < SIZE(all_special_pokemon); i++) {
      if (species == all_special_pokemon[i]) {
        return false;
      }
    }

    return HookManager::Call<bool>(HookID::kAddPokemonToTeam, team, pokemon);
  }

  static void LoadEvolveTableHook(u32 species, u32 b, u32 c, u32 d) {
    HookManager::Call<void>(HookID::kLoadEvolveTable, species, b, c, d);
    struct Table {
      void* vtable;
      u16 species;
      global_data::Evolve* evolve;
    }* table = (Table*)READ(u32, 0x617A04 + 0x34);
    auto& evolve = *table->evolve;
    switch (species) {
      case SPECIES_MACHOKE:
      case SPECIES_GRAVELER:
      case SPECIES_KADABRA:
      case SPECIES_BOLDORE:
      case SPECIES_GURDURR:
      case SPECIES_HAUNTER:
      case SPECIES_KARRABLAST:
      case SPECIES_SHELMET:
        evolve.data[0].arg = 30; // level
        evolve.data[0].method = EVOLUTION_METHOD_LEVEL_UP;
        break;
      case SPECIES_SLOWPOKE:
      case SPECIES_POLIWHIRL:
        evolve.data[0].arg = ITEM_KINGS_ROCK;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_SCYTHER:
      case SPECIES_ONIX:
        evolve.data[0].arg = ITEM_METAL_COAT;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_SEADRA:
        evolve.data[0].arg = ITEM_DRAGON_SCALE;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_PORYGON:
        evolve.data[0].arg = ITEM_UP_GRADE;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_PORYGON2:
        evolve.data[0].arg = ITEM_DUBIOUS_DISC;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_CLAMPERL:
        evolve.data[0].arg = ITEM_DEEP_SEA_TOOTH;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        evolve.data[1].arg = ITEM_DEEP_SEA_SCALE;
        evolve.data[1].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_RHYDON:
        evolve.data[0].arg = ITEM_PROTECTOR;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_ELECTABUZZ:
        evolve.data[0].arg = ITEM_ELECTIRIZER;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_MAGMAR:
        evolve.data[0].arg = ITEM_MAGMARIZER;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_DUSCLOPS:
        evolve.data[0].arg = ITEM_REAPER_CLOTH;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_FEEBAS:
        evolve.data[0].arg = ITEM_PRISM_SCALE;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_SPRITZEE:
        evolve.data[0].arg = ITEM_SACHET;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_SWIRLIX:
        evolve.data[0].arg = ITEM_WHIPPED_DREAM;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
      case SPECIES_PHANTUMP:
      case SPECIES_PUMPKABOO:
        evolve.data[0].arg = ITEM_LEAF_STONE;
        evolve.data[0].method = EVOLUTION_METHOD_ITEM;
        break;
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

  STATIC_INLINE void FixNickname() {
    static const c16* NICKNAME = u"STEVEN";
    auto& status = savedata::TrainerStatus::GetInstance();
    for (u32 i = 0; i < savedata::TrainerStatus::kPlayerNameLen; i++) {
      status.name[i] = status.nickname[i] = NICKNAME[i];
      if (NICKNAME[i] == '\0') break;
    }
  }

  static const EncounterEntry* GetEncounterEntry(u16 map_id);

  static void FixEncounterTable(overworld::EncounterData* data);
};
} // namespace feature