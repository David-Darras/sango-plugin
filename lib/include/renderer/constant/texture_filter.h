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
 * @brief The color filters of the 3D Pokémon models.
 */

#pragma once

#include <types.h>

namespace renderer {

/// A color filter of the 3D Pokémon models. See renderer::ModelFilter. The
/// order is the order of the list of the menu.
enum class TextureFilter : u8 {
  kNormal, ///< RGB: the colors of the game.
  kGrb, ///< The red and the green change places.
  kBgr,
  kRbg,
  kGbr,
  kBrg,
  kInverted, ///< (1 - R) (1 - G) (1 - B).
  kInvertedGrb,
  kInvertedBgr,
  kDarker,
  kBrighter,
  kAllBlack,
  kAllWhite,
  kGrayscale, ///< Black and white.
  kSepia,
  kLightOnly, ///< Only the light of the scene, without the textures.
  kSpecularOnly, ///< Only the shiny parts.
  kRimLight,
  kLiquidMetal,
  kGhost, ///< The model is half transparent.
  kCount,
};

} // namespace renderer
