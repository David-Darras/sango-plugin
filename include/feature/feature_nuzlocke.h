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
#include "kaizo.h"
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

namespace feature {
struct Nuzlocke {
  MAKE_SINGLETON(Nuzlocke)
  uptr archive_buffer = 0;
  u32* size = nullptr;
  u32 trainer_id = 0;

  STATIC_INLINE void Initialize() {
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
                                   uptr pkm) {
#ifdef KAIZO
    kaizo::PatchStarter(pkm);
#endif
    return HookManager::Call<
      u32>(HookID::kInitializePokemon, param, heap, pkm);
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

  static bool CreateOverworldModelsHook(uptr man, ModelData* table, u32 max,
                                        u16 map_id,
                                        void** fashion) {
    return HookManager::Call<bool>(HookID::kCreateOverworldModels, man, table,
                                   max, map_id, fashion);
  }

  static bool AddPokemonToTeamHook(savedata::PokemonTeam* team,
                                   savedata::PokemonParam* pokemon) {
#ifdef KAIZO
    if (team == &savedata::PokemonTeam::GetInstance()) {
      pokemon->accessor->Decrypt();
      u16 species = pokemon->core->species;
      pokemon->accessor->Encrypt();

      if (kaizo::IsSpecialPokemon(species)) {
        return false;
      }
    }
#endif

    return HookManager::Call<bool>(HookID::kAddPokemonToTeam, team, pokemon);
  }

  static void LoadEvolveTableHook(u32 species, u32 b, u32 c, u32 d) {
    HookManager::Call<void>(HookID::kLoadEvolveTable, species, b, c, d);
#ifdef KAIZO
    kaizo::PatchEvolve(species);
#endif
  }
};
} // namespace feature