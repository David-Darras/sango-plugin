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
 * @file texture_filter.h
 * @brief The color filters of the Pokémon models in battle.
 */

#pragma once

#include <types.h>

namespace renderer {

/// A color filter of the Pokémon models in battle. See renderer::ModelFilter.
enum class TextureFilter : u8 {
  kNormal,
  kPitchBlack,
  kInvert,
  kDarken,
  kOverexposed,
  kPsychedelic,
  kSepia,
  kObsidian,
  kPlasma,
  kSketch,
  kChromeMetallic,
  kLiquidChrome,
  kGhost,
  kCount,
};

} // namespace renderer
