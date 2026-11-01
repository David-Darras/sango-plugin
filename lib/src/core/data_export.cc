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
 * @file data_export.cc
 * @brief Writes data of the game in the format of the files of Sango Maker.
 *
 * The declarations are in core/data_export.h.
 */

#include "core/data_export.h"

#include <cstdarg>
#include <cstdio>

#include "core/data_folder.h"
#include "core/maker_reader.h"
#include "core/native/process_manager.h"
#include "overworld/native/common_resource.h"
#include "overworld/native/encounter_data.h"
#include "overworld/native/map_manager.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/edit_pokemon.h"
#include "pokemon/native/species_data.h"
#include "savedata/native/pokemon_team.h"
#include "system/native/file.h"

namespace core {
namespace {

// A text that grows, then goes to a file of the data folder.
class TextWriter {
public:
  // constexpr: the writer needs no constructor at the start, so the products
  // that do not export do not contain it.
  constexpr TextWriter() : text_(), used_(0) {}

  void Clear() { used_ = 0; }

  void Add(const c8* format, ...) {
    if (used_ >= sizeof(text_)) return;
    va_list args;
    va_start(args, format);
    const s32 length =
        std::vsnprintf(text_ + used_, sizeof(text_) - used_, format, args);
    va_end(args);
    if (length > 0) used_ += static_cast<u32>(length);
    if (used_ > sizeof(text_) - 1) used_ = sizeof(text_) - 1;
  }

  // The name of a value, or its number when it has no name.
  void AddName(NameKind kind, u32 value) {
    const c8* name = DataNames::GetName(kind, value);
    if (name != nullptr) {
      Add("%s", name);
    } else {
      Add("%u", value);
    }
  }

  bool WriteTo(const c16* path) const {
    sys::File file(path, true);
    if (!file.IsOpen()) return false;
    file.Write(text_, used_);
    return true;
  }

private:
  c8 text_[8 * 1024];
  u32 used_;
};

// One text for all the exports: the menu writes one file at a time.
TextWriter writer;

} // namespace

bool DataExport::WriteEncounters() {
#ifdef GAME_ORAS
  if (!ProcessManager::IsOverworldActive()) return false;
  const MapId map = overworld::MapManager::GetInstance().GetMap();
  overworld::EncounterData& data =
      overworld::CommonResource::GetInstance().GetDexNavData(
          static_cast<u16>(map));
  writer.Clear();
  writer.Add("# The wild Pokemon of the map %u. Copy them to encounters.txt.\n",
             static_cast<u32>(map));
  for (u32 m = 0; m < static_cast<u32>(EncounterMethod::kMax); m++) {
    const EncounterMethod method = static_cast<EncounterMethod>(m);
    if (data.GetRate(method) == 0) continue;
    u32 count = 0;
    const overworld::PokeInfoOnAction* places =
        overworld::WildEncounter::GetPlaces(&data, method, &count);
    if (places == nullptr) continue;
    writer.Add("\n[");
    writer.AddName(NameKind::kMap, static_cast<u32>(map));
    writer.Add(", ");
    writer.AddName(NameKind::kEncounterMethod, m);
    writer.Add("]\n");
    for (u32 i = 0; i < count; i++) {
      const overworld::PokeInfoOnAction& place = places[i];
      writer.AddName(NameKind::kSpecies, static_cast<u32>(place.species));
      writer.Add(", %u, %u\n", place.min_level, place.max_level);
    }
  }
  return writer.WriteTo(SANGO_DATA_PATH(u"export_encounters.txt"));
#else
  return false;
#endif
}

bool DataExport::WriteSpecies(SpeciesId species) {
  const pokemon::SpeciesData& data = pokemon::SpeciesData::GetInstance(species);
  writer.Clear();
  writer.Add("# Copy the species to species.txt.\n[");
  writer.AddName(NameKind::kSpecies, static_cast<u32>(species));
  writer.Add("]\nstats = %u, %u, %u, %u, %u, %u\ntypes = ", data.base_hp,
             data.base_attack, data.base_defense, data.base_special_attack,
             data.base_special_defense, data.base_speed);
  writer.AddName(NameKind::kType, static_cast<u32>(data.type[0]));
  writer.Add(", ");
  writer.AddName(NameKind::kType, static_cast<u32>(data.type[1]));
  writer.Add("\nabilities = ");
  for (u32 i = 0; i < 3; i++) {
    if (i != 0) writer.Add(", ");
    writer.AddName(NameKind::kAbility, static_cast<u32>(data.ability[i]));
  }
  writer.Add("\ncatch rate = %u\n", data.capture_rate);
  return writer.WriteTo(SANGO_DATA_PATH(u"export_species.txt"));
}

bool DataExport::WriteTeam() {
  savedata::PokemonTeam& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return false;
  writer.Clear();
  writer.Add("# The team of the player. Copy the lines below a [trainer] of "
             "trainers.txt.\n");
  for (u32 slot = 0; slot < team.count; slot++) {
    savedata::PokemonParam* param = team.Get(slot);
    if (param == nullptr) continue;
    // The level is in the calculated data: it is encrypted with the data.
    pokemon::EditPokemon edit(*param, false);
    const pokemon::CoreData& core = *edit;
    const pokemon::RuntimeData* runtime = edit.GetRuntimeData();
    writer.Add("pokemon = ");
    writer.AddName(NameKind::kSpecies, static_cast<u32>(core.species));
    writer.Add(", level %u", runtime != nullptr ? runtime->level : 50);
    if (core.item != ItemId::kNone) {
      writer.Add(", item ");
      writer.AddName(NameKind::kItem, static_cast<u32>(core.item));
    }
    writer.Add(", ability ");
    writer.AddName(NameKind::kAbility, static_cast<u32>(core.ability));
    writer.Add(", nature ");
    writer.AddName(NameKind::kNature, static_cast<u32>(core.nature));
    writer.Add(", evs %u/%u/%u/%u/%u/%u", core.ev_hp, core.ev_attack,
               core.ev_defense, core.ev_special_attack,
               core.ev_special_defense, core.ev_speed);
    writer.Add(", moves ");
    bool is_first = true;
    for (u32 i = 0; i < 4; i++) {
      if (core.moves[i] == MoveId::kNone) continue;
      if (!is_first) writer.Add(" / ");
      writer.AddName(NameKind::kMove, static_cast<u32>(core.moves[i]));
      is_first = false;
    }
    writer.Add("\n");
  }
  return writer.WriteTo(SANGO_DATA_PATH(u"export_team.txt"));
}

} // namespace core
