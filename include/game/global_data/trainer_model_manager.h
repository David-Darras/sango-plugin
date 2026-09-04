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

#include "common.h"
#include "game/constant/trainer_model.h"

// Load Trainer Model : 0x00458214
class TrainerModelManager {
  SINGLETON(TrainerModelManager)
private:
  u16 assets[static_cast<u8>(TrainerModel::kCount)];
  u16 textures[static_cast<u8>(TrainerModel::kCount)];
  u16 battle_animations[static_cast<u8>(TrainerModel::kCount)];
  u16 idle_animations[static_cast<u8>(TrainerModel::kCount)];
  u16 cinematic_animations[static_cast<u8>(TrainerModel::kCount)];

public:
  STATIC_INLINE TrainerModelManager& GetInstance() {
    return *(TrainerModelManager*)ADDRESS_TRAINER_MODEL_TABLE;
  }

  INLINE void Replace(TrainerModel src, TrainerModel dst) {
    const u8 s = static_cast<u8>(src);
    const u8 d = static_cast<u8>(dst);
    assets[s] = 0 + d * 5;
    textures[s] = 1 + d * 5;
    battle_animations[s] = 2 + d * 5;
    idle_animations[s] = 3 + d * 5;
    // NO TPOSE
    cinematic_animations[s] = 4 + s * 5;
  }
};