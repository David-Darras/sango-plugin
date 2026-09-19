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

#include <cstddef>

#include "core/types.h"
#include "overworld/native/model_appearance.h"

namespace overworld {

constexpr u32 kMaxModelsPerRegion = 33;

/// The game's field character manager; only the tail holding the model list
/// of the current region is mapped (the model list setup stage sits at
/// offset 13994).
struct CharacterManager {
  static constexpr u32 kArchiveOffset = GAME_CONSTANT(0x14A8, 14004);
  u8 _0[kArchiveOffset];
  void* model_param_archive;
  u8 _1[12];
  u32 model_count_original;
  u32 model_count;
  ModelAppearance models[kMaxModelsPerRegion];
};

static_assert(offsetof(CharacterManager, model_param_archive) == CharacterManager::kArchiveOffset &&
              offsetof(CharacterManager, model_count_original) == CharacterManager::kArchiveOffset + 16 &&
              offsetof(CharacterManager, model_count) == CharacterManager::kArchiveOffset + 20 &&
              offsetof(CharacterManager, models) == CharacterManager::kArchiveOffset + 24,
              "CharacterManager must match the game's field character manager layout");

} // namespace overworld
