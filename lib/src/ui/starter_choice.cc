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

#include "ui/patch/starter_choice.h"

namespace ui {

void StarterChoice::PatchLoad() {
  auto& feat = GetInstance();
  if (feat.candidates[0] == SpeciesId::kNone) return;
  for (u32 i = 0; i < kCount; i++) {
    Apply((PokeInfo*)(core::address::kStarter + kViewStride * i), feat.candidates[i]);
    Apply((PokeInfo*)(core::address::kStarterModel + kModelStride * i),
          feat.candidates[i]);
  }
}

void StarterChoice::Apply(PokeInfo* info, SpeciesId species) {
  info->species = species;
  info->form = Form::kNormal;
  info->gender = Gender::kMale;
  info->is_shiny = false;
  info->is_egg = false;
}

} // namespace ui
