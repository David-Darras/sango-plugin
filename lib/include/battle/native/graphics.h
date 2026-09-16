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
#include "battle/native/base_model.h"
#include "battle/native/environment.h"
#include "battle/native/manager.h"

namespace overworld {
struct StereoCamera;
}

namespace battle {

class Graphics {
  SINGLETON(Graphics)
public:
  STATIC_INLINE Graphics& GetInstance() {
    return Manager::GetInstance().GetGraphics();
  }

  overworld::StereoCamera& GetStereoCamera() {
    return *(overworld::StereoCamera*)((uptr)this + 0x1F0);
  }

  BaseModel& GetPokemonModel(u32 index) const {
    if (index >= 6) index = 5;
    return *pokemon_model[index];
  }

  BaseModel& GetTrainerModel(u32 index) const {
    if (index >= 4) index = 3;
    return *trainer_model[index];
  }

  u8 _0[0x100];
  BaseModel* pokemon_model[6];
  BaseModel* trainer_model[4];
  Environment* environment;
  u32 _1;
  Bundle* platform_pack_0;
  Bundle* platform_pack_1;
  void* platform_resource[7];
  BaseModel* platform_model[6];
  void* big_platform_resource[2];
  BaseModel* big_platform_model[4];
  u32 _2;
  BaseModel* prop_model;
  BaseModel* unknow;
};

} // namespace battle
