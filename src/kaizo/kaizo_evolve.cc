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
#include "feature/hook_manager.h"
#include "game/constant/evolution_method.h"
#include "game/constant/item.h"
#include "game/constant/species.h"
#include "game/global_data/evolve.h"

namespace global_data {
struct Evolve;
}

namespace kaizo {
struct Table {
  void* vtable;
  u16 species;
  global_data::Evolve* evolve;
};

void PatchEvolve(u16 species) {
  Table* table = (Table*)READ32(0x617A04 + 0x34);
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

static void LoadEvolveTableHook(u32 species, u32 b, u32 c, u32 d) {
  HookManager::Call<void>(HookID::kLoadEvolveTable, species, b, c, d);
  PatchEvolve(species);
}

void InitializeEvolveHook() {
  HookManager::Initialize(HookID::kLoadEvolveTable,
                          ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE,
                          (uptr)LoadEvolveTableHook
      );
}
}