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

struct ZaMega {
  SpeciesId species;
  u8 form;
  ItemId item;
  const c8* name;
};

static const ZaMega kZaMegas[] = {
  {SpeciesId::kRaichu, 2, ItemId::kChoiceBand, "Raichu Y"},
  {SpeciesId::kRaichu, 3, ItemId::kChoiceSpecs, "Raichu X"},
  {SpeciesId::kClefable, 1, ItemId::kLifeOrb, "Clefable"},
  {SpeciesId::kVictreebel, 1, ItemId::kLifeOrb, "Victreebel"},
  {SpeciesId::kStarmie, 1, ItemId::kLifeOrb, "Starmie"},
  {SpeciesId::kDragonite, 1, ItemId::kLifeOrb, "Dragonite"},
  {SpeciesId::kMeganium, 1, ItemId::kLifeOrb, "Meganium"},
  {SpeciesId::kFeraligrator, 1, ItemId::kLifeOrb, "Feraligrator"},
  {SpeciesId::kSkarmory, 1, ItemId::kLifeOrb, "Skarmory"},
  {SpeciesId::kChimecho, 1, ItemId::kLifeOrb, "Chimecho"},
  {SpeciesId::kAbsol, 2, ItemId::kLifeOrb, "Absol Z"},
  {SpeciesId::kStaraptor, 1, ItemId::kLifeOrb, "Staraptor"},
  {SpeciesId::kGarchomp, 2, ItemId::kLifeOrb, "Garchomp Z"},
  {SpeciesId::kLucario, 2, ItemId::kLifeOrb, "Lucario Z"},
  {SpeciesId::kFroslass, 1, ItemId::kLifeOrb, "Froslass"},
  {SpeciesId::kHeatran, 1, ItemId::kLifeOrb, "Heatran"},
  {SpeciesId::kDarkrai, 1, ItemId::kLifeOrb, "Darkrai"},
  {SpeciesId::kEmboar, 1, ItemId::kLifeOrb, "Emboar"},
  {SpeciesId::kExcadrill, 1, ItemId::kLifeOrb, "Excadrill"},
  {SpeciesId::kScolipede, 1, ItemId::kLifeOrb, "Scolipede"},
  {SpeciesId::kScrafty, 1, ItemId::kLifeOrb, "Scrafty"},
  {SpeciesId::kEelektross, 1, ItemId::kLifeOrb, "Eelektross"},
  {SpeciesId::kChandelure, 1, ItemId::kLifeOrb, "Chandelure"},
  {SpeciesId::kGolurk, 1, ItemId::kLifeOrb, "Golurk"},
  {SpeciesId::kChesnaught, 1, ItemId::kLifeOrb, "Chesnaught"},
  {SpeciesId::kDelphox, 1, ItemId::kLifeOrb, "Delphox"},
  {SpeciesId::kGreninja, 1, ItemId::kLifeOrb, "Greninja"},
  {SpeciesId::kPyroar, 1, ItemId::kLifeOrb, "Pyroar"},
  {SpeciesId::kFloette, 6, ItemId::kLifeOrb, "Floette"},
  {SpeciesId::kMeowstic, 2, ItemId::kLifeOrb, "Meowstic"},
  {SpeciesId::kMalamar, 1, ItemId::kLifeOrb, "Malamar"},
  {SpeciesId::kBarbaracle, 1, ItemId::kLifeOrb, "Barbaracle"},
  {SpeciesId::kDragalge, 1, ItemId::kLifeOrb, "Dragalge"},
  {SpeciesId::kHawlucha, 1, ItemId::kLifeOrb, "Hawlucha"},
  {SpeciesId::kZygarde, 1, ItemId::kLifeOrb, "Zygarde"},
};
