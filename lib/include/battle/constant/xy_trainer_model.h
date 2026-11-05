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
 * @file xy_trainer_model.h
 * @brief The 3D models of the trainers of Pokémon X and Y that ORAS does not
 *        have.
 */

#pragma once

#include <types.h>

namespace battle {
/**
 * @brief A 3D model of a trainer of Pokémon X and Y, for the battles of ORAS.
 *
 * ORAS already has Serena, Calem, Malva, Holocaster, Shauna, Tierno and
 * Trevor (see TrainerModelId). These models are the others. They come from
 * the files of X and Y: tools/export_xy_trainer_models.py writes them in
 * sdmc:/sango/trainer_model. TrainerModelExtension shows them.
 */
enum class XyTrainerModelId : u8 {
  kSycamore = 0,
  kLysandre = 1, ///< The boss of Team Flare, with the suit.
  kFlareAdminMale = 2,
  kFlareAdminFemale = 3,
  kAliana = 4,
  kXerosic = 5,
  kFlareGruntMale = 6,
  kFlareGruntFemale = 7,
  kCelosia = 8,
  kBryony = 9,
  kMable = 10,
  kAz = 11,
  kLysandreFinal = 12, ///< Lysandre in his last battle.
  kCount = 13,
};
} // namespace battle

using battle::XyTrainerModelId;
