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

#include "savedata/pokedex.h"
#include "menu/plugin_menu.h"

namespace savedata {
#include "savedata/pokedex_form.inc"

static FORCE_INLINE void GetTableIndexAndFormMax(s32& table_idx, s32& form_max,
                                                 u16 species) {
  struct Entry {
    u16 species;
    u16 form_max;
    bool is_mega_evolution;
  };

  Entry* entry = (Entry*)ADDRESS_POKEDEX_FORM_TABLE;
  table_idx = 0;
  form_max = -1;
  while (entry->species != 0) {
    if (entry->species == species) {
      form_max = entry->form_max;
      return;
    }
    table_idx++;
    entry++;
  }
}

void Pokedex::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u16 species = 1;
  static u8 form = 0;
  static u16 prev_species = species;
  if (species != prev_species) {
    form = 0;
    prev_species = species;
  }

  u32 idx, bit_pos, array_idx;
  Pokedex& data = GetInstance();

  menu.Add("Spinda Pattern", data.spinda_pattern)
      .AddSeparator();

  menu.AddSpecies("Species", species)
      .WithBounds(1, 721)
      .WithRefresh()
      .AddSeparator();

  idx = data.GetFormIndex(species);
  if (idx != -1) {
    s32 table_index, form_max;
    GetTableIndexAndFormMax(table_index, form_max, species);
    if (form >= form_max) form = 0;

    idx += form;
    bit_pos = idx & 31;
    array_idx = idx >> 5;

    u32* normal_form_seen_flags = (u32*)data.form_seen_flags[0];
    u32* shiny_form_seen_flags = (u32*)data.form_seen_flags[1];

    menu.Add("Form", form)
        .WithArray(FORMS[table_index], form_max)
        .WithRefresh()
        .Add("Form Seen: Normal",
             &normal_form_seen_flags[array_idx], bit_pos,
             1)
        .Add("Form Seen: Shiny", &shiny_form_seen_flags[array_idx], bit_pos, 1);

    u32* normal_displayed_form_flags = (u32*)data.displayed_form_flags[0];
    u32* shiny_displayed_form_flags = (u32*)data.displayed_form_flags[1];

    menu.Add("Display Form: Normal", &normal_displayed_form_flags[array_idx],
             bit_pos, 1)
        .Add("Display Form: Shiny", &shiny_displayed_form_flags[array_idx],
             bit_pos, 1)
        .AddSeparator();
  }
  idx = species - 1;
  bit_pos = idx & 31;
  array_idx = idx >> 5;

  menu.Add("Captured", &data.captured_flags[array_idx], bit_pos, 1)
      .Add("Times Encountered", data.seen_count[species])
      .WithBounds(0, 999)
      .AddSeparator()
      .Add("Seen: Male", &data.gender_seen_flags[0][array_idx], bit_pos, 1)
      .Add("Seen: Female", &data.gender_seen_flags[1][array_idx], bit_pos, 1)
      .Add("Seen: Shiny Male", &data.gender_seen_flags[2][array_idx], bit_pos,
           1)
      .Add("Seen: Shiny Female", &data.gender_seen_flags[3][array_idx], bit_pos,
           1)
      .AddSeparator()
      .Add("Display: Male", &data.displayed_gender_flags[0][array_idx],
           bit_pos, 1)
      .Add("Display: Female", &data.displayed_gender_flags[1][array_idx],
           bit_pos, 1)
      .Add("Display: Shiny Male",
           &data.displayed_gender_flags[2][array_idx], bit_pos, 1)
      .Add("Display: Shiny Female",
           &data.displayed_gender_flags[3][array_idx], bit_pos, 1);
}
} // namespace savedata