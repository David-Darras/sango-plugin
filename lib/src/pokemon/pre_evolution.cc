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
 * @file pre_evolution.cc
 * @brief Gives the species that a species evolves from. See
 * pokemon/patch/pre_evolution.h.
 */

#include "pokemon/patch/pre_evolution.h"

#include "pokemon/native/evolution_data.h"
#include "pokemon/patch/species_table.h"

namespace pokemon {
namespace {

// The species before each species of the game (1 to 721), read from the
// evolutions of the game. 0 means that the species has no species before it.
constexpr u16 kGameSpeciesCount = 722;
const u16 kPreEvolution[kGameSpeciesCount] = {
    0, 0, 1, 2, 0, 4, 5, 0, 7, 8, 0, 10, 11, 0, 13, 14,
    0, 16, 17, 0, 19, 0, 21, 0, 23, 172, 25, 0, 27, 0, 29, 30,
    0, 32, 33, 173, 35, 0, 37, 174, 39, 0, 41, 0, 43, 44, 0, 46,
    0, 48, 0, 50, 0, 52, 0, 54, 0, 56, 0, 58, 0, 60, 61, 0,
    63, 64, 0, 66, 67, 0, 69, 70, 0, 72, 0, 74, 75, 0, 77, 0,
    79, 0, 81, 0, 0, 84, 0, 86, 0, 88, 0, 90, 0, 92, 93, 0,
    0, 96, 0, 98, 0, 100, 0, 102, 0, 104, 236, 236, 0, 0, 109, 0,
    111, 440, 0, 0, 0, 116, 0, 118, 0, 120, 439, 0, 238, 239, 240, 0,
    0, 0, 129, 0, 0, 0, 133, 133, 133, 0, 0, 138, 0, 140, 0, 446,
    0, 0, 0, 0, 147, 148, 0, 0, 0, 152, 153, 0, 155, 156, 0, 158,
    159, 0, 161, 0, 163, 0, 165, 0, 167, 42, 0, 170, 0, 0, 0, 0,
    175, 0, 177, 0, 179, 180, 44, 298, 183, 438, 61, 0, 187, 188, 0, 0,
    191, 0, 0, 194, 133, 133, 0, 79, 0, 0, 360, 0, 0, 204, 0, 0,
    95, 0, 209, 0, 123, 0, 0, 0, 0, 216, 0, 218, 0, 220, 0, 0,
    223, 0, 458, 0, 0, 228, 117, 0, 231, 137, 0, 0, 0, 236, 0, 0,
    0, 0, 113, 0, 0, 0, 0, 246, 247, 0, 0, 0, 0, 252, 253, 0,
    255, 256, 0, 258, 259, 0, 261, 0, 263, 0, 265, 266, 265, 268, 0, 270,
    271, 0, 273, 274, 0, 276, 0, 278, 0, 280, 281, 0, 283, 0, 285, 0,
    287, 288, 0, 290, 0, 0, 293, 294, 0, 296, 0, 0, 0, 300, 0, 0,
    0, 304, 305, 0, 307, 0, 309, 0, 0, 0, 0, 406, 0, 316, 0, 318,
    0, 320, 0, 322, 0, 0, 325, 0, 0, 328, 329, 0, 331, 0, 333, 0,
    0, 0, 0, 0, 339, 0, 341, 0, 343, 0, 345, 0, 347, 0, 349, 0,
    0, 0, 353, 0, 355, 0, 433, 0, 0, 0, 361, 0, 363, 364, 0, 366,
    366, 0, 0, 0, 371, 372, 0, 374, 375, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 387, 388, 0, 390, 391, 0, 393, 394, 0, 396, 397, 0,
    399, 0, 401, 0, 403, 404, 0, 315, 0, 408, 0, 410, 0, 412, 412, 0,
    415, 0, 0, 418, 0, 420, 0, 422, 190, 0, 425, 0, 427, 200, 198, 0,
    431, 0, 0, 434, 0, 436, 0, 0, 0, 0, 0, 0, 443, 444, 0, 0,
    447, 0, 449, 0, 451, 0, 453, 0, 0, 456, 0, 0, 459, 215, 82, 108,
    112, 114, 125, 126, 176, 193, 133, 133, 207, 221, 233, 281, 299, 356, 361, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    495, 496, 0, 498, 499, 0, 501, 502, 0, 504, 0, 506, 507, 0, 509, 0,
    511, 0, 513, 0, 515, 0, 517, 0, 519, 520, 0, 522, 0, 524, 525, 0,
    527, 0, 529, 0, 0, 532, 533, 0, 535, 536, 0, 0, 0, 540, 541, 0,
    543, 544, 0, 546, 0, 548, 0, 0, 551, 552, 0, 554, 0, 0, 557, 0,
    559, 0, 0, 562, 0, 564, 0, 566, 0, 568, 0, 570, 0, 572, 0, 574,
    575, 0, 577, 578, 0, 580, 0, 582, 583, 0, 585, 0, 0, 588, 0, 590,
    0, 592, 0, 0, 595, 0, 597, 0, 599, 600, 0, 602, 603, 0, 605, 0,
    607, 608, 0, 610, 611, 0, 613, 0, 0, 616, 0, 0, 619, 0, 0, 622,
    0, 624, 0, 0, 627, 0, 629, 0, 0, 0, 633, 634, 0, 636, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 650, 651, 0, 653, 654,
    0, 656, 657, 0, 659, 0, 661, 662, 0, 664, 665, 0, 667, 0, 669, 670,
    0, 672, 0, 674, 0, 0, 677, 0, 679, 680, 0, 682, 0, 684, 0, 686,
    0, 688, 0, 690, 0, 692, 0, 694, 0, 696, 0, 698, 133, 0, 0, 0,
    0, 704, 705, 0, 0, 708, 0, 710, 0, 712, 0, 714, 0, 0, 0, 0,
    0, 0,
};

// The species that the plugin adds (722 and after): the library has their
// evolutions, so the table is built from them the first time.
constexpr u32 kExtraLimit = 1100;
u16 extra_pre_evolution[kExtraLimit];
bool is_extra_built = false;

void BuildExtraTable() {
  is_extra_built = true;
  const u32 last = SpeciesTable::kFirstGen8Species + SpeciesTable::kGen8Count;
  for (u32 from = kGameSpeciesCount; from < last && from < kExtraLimit; from++) {
    EvolutionData table;
    if (!SpeciesTable::PatchEvolutionTable(static_cast<u16>(from), &table)) {
      continue;
    }
    for (const auto& entry : table.data) {
      const u32 to = static_cast<u32>(entry.species);
      if (entry.method == EvolutionMethod::kNone || to == 0 ||
          to >= kExtraLimit || to == from) {
        continue;
      }
      if (extra_pre_evolution[to] == 0) {
        extra_pre_evolution[to] = static_cast<u16>(from);
      }
    }
  }
}

} // namespace

SpeciesId PreEvolution::Get(SpeciesId species) {
  const u32 index = static_cast<u32>(species);
  if (index < kGameSpeciesCount) return static_cast<SpeciesId>(kPreEvolution[index]);
  if (index >= kExtraLimit) return SpeciesId::kNone;
  if (!is_extra_built) BuildExtraTable();
  return static_cast<SpeciesId>(extra_pre_evolution[index]);
}

} // namespace pokemon
