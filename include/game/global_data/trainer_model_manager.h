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
  u16 assets[TRAINER_MODEL_COUNT];
  u16 textures[TRAINER_MODEL_COUNT];
  u16 battle_animations[TRAINER_MODEL_COUNT];
  u16 idle_animations[TRAINER_MODEL_COUNT];
  u16 cinematic_animations[TRAINER_MODEL_COUNT];

public:
  STATIC_INLINE TrainerModelManager& GetInstance() {
    return *(TrainerModelManager*)ADDRESS_TRAINER_MODEL_TABLE;
  }

  INLINE void Replace(u8 src, u8 dst) {
    assets[src] = 0 + dst * 5;
    textures[src] = 1 + dst * 5;
    battle_animations[src] = 2 + dst * 5;
    idle_animations[src] = 3 + dst * 5;
    // NO TPOSE
    cinematic_animations[src] = 4 + src * 5;
  }
};