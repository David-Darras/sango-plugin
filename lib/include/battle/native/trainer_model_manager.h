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
 * @file trainer_model_manager.h
 * @brief The table of the trainer models in battle.
 */

#pragma once

#include "common.h"
#include "battle/constant/trainer_model.h"

namespace battle {

/// The table of the files of the trainer models in battle.
class TrainerModelManager {
  SINGLETON(TrainerModelManager)
private:
  u16 assets[static_cast<u8>(TrainerModelId::kCount)];
  u16 textures[static_cast<u8>(TrainerModelId::kCount)];
  u16 battle_animations[static_cast<u8>(TrainerModelId::kCount)];
  u16 idle_animations[static_cast<u8>(TrainerModelId::kCount)];
  u16 cinematic_animations[static_cast<u8>(TrainerModelId::kCount)];

public:
  STATIC_INLINE TrainerModelManager& GetInstance() {
    return *(TrainerModelManager*)core::address::kTrainerModelTable;
  }

  /// Shows the model `dst` instead of the model `src`.
  INLINE void Replace(TrainerModelId src, TrainerModelId dst) {
    const u8 s = static_cast<u8>(src);
    const u8 d = static_cast<u8>(dst);
    assets[s] = 0 + d * 5;
    textures[s] = 1 + d * 5;
    battle_animations[s] = 2 + d * 5;
    idle_animations[s] = 3 + d * 5;
    cinematic_animations[s] = 4 + s * 5;
  }
};

} // namespace battle
