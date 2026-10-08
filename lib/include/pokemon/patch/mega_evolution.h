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
 * @file mega_evolution.h
 * @brief Adds Mega Evolutions to species.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"

namespace pokemon {

/// The form of Mega Mime Jr. It is a form of the plugin, not of the game:
/// it is a constant, not a value of FormId.
constexpr FormId kFormMimeJrMega = static_cast<FormId>(10);

/// Changes the Mega Evolution data when the game loads it.
class MegaEvolution {
  MAKE_SINGLETON(MegaEvolution)

public:
  static void Initialize();

private:
  static void LoadMegaEvolutionTableHook(SpeciesId species);
};
} // namespace pokemon
