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

/**
 * @file evolution.cc
 * @brief Adds the evolutions of the new species to the evolution table.
 *
 * The declarations are in pokemon/patch/evolution.h.
 */

#include "pokemon/patch/evolution.h"
#include "battle/patch/item_extension.h"
#include "core/hook.h"
#include "pokemon/patch/species_editor.h"
#include "pokemon/patch/species_table.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/constant/item.h"
#include "pokemon/native/database.h"
#include "pokemon/native/evolution_table.h"
#include "pokemon/address.h"
#include "system/native/file.h"

namespace pokemon {

namespace {
core::Hook<void(SpeciesId, u32, u32, u32)> load_evolve_table_hook;
core::Hook<void(void*, u32, u32)> evolve_hook;

/// The number of places in the evolution table of a species.
constexpr u32 kEvolutionPlaces = 8;
/// The value of a place that keeps the form of the Pokémon.
constexpr u8 kKeepForm = 0xFF;

/// Writes a number in a path. Returns the place after it.
u32 PutNumber(c16* out, u32 at, u32 number) {
  c16 digits[6];
  u32 count = 0;
  do {
    digits[count++] = static_cast<c16>(u'0' + number % 10);
    number /= 10;
  } while (number != 0);
  while (count > 0) out[at++] = digits[--count];
  return at;
}

/// The species and the form of a Pokémon.
void SpeciesAndForm(void* pokemon, u32* species, u32* form) {
  const uptr data = READ32(reinterpret_cast<uptr>(pokemon) + 12);
  *species = core::GameFunction<u32(uptr)>::Call(address::kAccessorGetSpecies, data) & 0xFFFF;
  *form = core::GameFunction<u32(uptr)>::Call(address::kAccessorGetForm, data) & 0xFF;
}
} // namespace

void Evolution::Initialize() {
  load_evolve_table_hook.Install(address::kLoadEvolutionTable,
                                 LoadEvolveTableHook);
  evolve_hook.Install(address::kCoreEvolve, EvolveHook);
}

void Evolution::EvolveHook(void* pokemon, u32 next_species, u32 place) {
  u32 species = 0;
  u32 form = 0;
  SpeciesAndForm(pokemon, &species, &form);
  evolve_hook(pokemon, next_species, place);
  if (place >= kEvolutionPlaces) return;
  static const c16 kFolder[] = u"sdmc:/sango-studio/species/evolution_form/";
  c16 path[80];
  u32 at = 0;
  for (const c16* c = kFolder; *c != 0; c++) path[at++] = *c;
  at = PutNumber(path, at, species);
  path[at++] = u'_';
  at = PutNumber(path, at, form);
  for (const c16* c = u".bin"; *c != 0; c++) path[at++] = *c;
  path[at] = 0;
  u8 forms[kEvolutionPlaces] = {};
  if (sys::File::ReadAll(path, forms, sizeof(forms)) != sizeof(forms)) return;
  const u8 target = forms[place & 0xFF];
  if (target == kKeepForm) return;
  core::GameFunction<void(void*, u32)>::Call(address::kCoreChangeForm, pokemon,
                                             target);
}

void Evolution::LoadEvolveTableHook(SpeciesId species, u32 b, u32 c, u32 d) {
  const bool extra = SpeciesTable::IsAddedSpecies(static_cast<u16>(species));
  load_evolve_table_hook(extra ? static_cast<SpeciesId>(1) : species, b, c, d);
  PatchEvolve(species);
  battle::ItemExtension::PatchEvolution(species,
                                        *Database::GetInstance().evolution->data);
  // The evolutions of the product come last: they replace the others.
  SpeciesChanges::PatchEvolutions(species,
                                  *Database::GetInstance().evolution->data);
}

void Evolution::PatchEvolve(SpeciesId species) {
  auto& evolve_table = *Database::GetInstance().evolution;
  auto& table = *evolve_table.data;
  if (SpeciesTable::PatchEvolutionTable(static_cast<u16>(species), &table)) {
    return;
  }
  switch (species) {
    case SpeciesId::kMachoke:
    case SpeciesId::kGraveler:
    case SpeciesId::kKadabra:
    case SpeciesId::kBoldore:
    case SpeciesId::kGurdurr:
    case SpeciesId::kHaunter:
    case SpeciesId::kKarrablast:
    case SpeciesId::kShelmet:
      table.data[0].level = 30;
      table.data[0].method = EvolutionMethod::kLevelUp;
      break;
    case SpeciesId::kSlowpoke:
    case SpeciesId::kPoliwhirl:
      table.data[0].item = ItemId::kKingsRock;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kScyther:
    case SpeciesId::kOnix:
      table.data[0].item = ItemId::kMetalCoat;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kSeadra:
      table.data[0].item = ItemId::kDragonScale;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kPorygon:
      table.data[0].item = ItemId::kUpGrade;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kPorygon2:
      table.data[0].item = ItemId::kDubiousDisc;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kClamperl:
      table.data[0].item = ItemId::kDeepSeaTooth;
      table.data[0].method = EvolutionMethod::kItem;
      table.data[1].item = ItemId::kDeepSeaScale;
      table.data[1].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kRhydon:
      table.data[0].item = ItemId::kProtector;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kElectabuzz:
      table.data[0].item = ItemId::kElectirizer;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kMagmar:
      table.data[0].item = ItemId::kMagmarizer;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kDusclops:
      table.data[0].item = ItemId::kReaperCloth;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kFeebas:
      table.data[0].item = ItemId::kPrismScale;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kSpritzee:
      table.data[0].item = ItemId::kSachet;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kSwirlix:
      table.data[0].item = ItemId::kWhippedDream;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kPhantump:
    case SpeciesId::kPumpkaboo:
      table.data[0].item = ItemId::kLeafStone;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    default:
      break;
  }
}

} // namespace pokemon
