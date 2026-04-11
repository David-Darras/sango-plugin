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

#include "savedata/pokemon_data_accessor.h"

#include <cstring>

#include "common.h"
#include "core/core.h"
#include "savedata/pokemon_core_data.h"

static const u8 block_positions[32][4] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 3, 1, 2}, {0, 2, 3, 1},
    {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2}, {2, 0, 1, 3}, {3, 0, 1, 2},
    {2, 0, 3, 1}, {3, 0, 2, 1}, {1, 2, 0, 3}, {1, 3, 0, 2}, {2, 1, 0, 3},
    {3, 1, 0, 2}, {2, 3, 0, 1}, {3, 2, 0, 1}, {1, 2, 3, 0}, {1, 3, 2, 0},
    {2, 1, 3, 0}, {3, 1, 2, 0}, {2, 3, 1, 0}, {3, 2, 1, 0}, {0, 1, 2, 3},
    {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 3, 1, 2}, {0, 2, 3, 1}, {0, 3, 2, 1},
    {1, 0, 2, 3}, {1, 0, 3, 2},
};

u8 PokemonDataAccessor::GetBlockPosition(u32 encryption_key, u32 index) {
  return block_positions[(encryption_key & (0b11111 << 13)) >> 13][index];
}

void PokemonDataAccessor::UnshuffleBlocks() {
  PokemonCoreData data;
  std::memcpy(&data, pkm_core_data_, sizeof(data));

  for (u32 i = 0; i < PokemonCoreData::kBlockCount; i++) {
    std::memcpy(
        pkm_core_data_->GetBlock(i),
        data.GetBlock(GetBlockPosition(pkm_core_data_->encryption_key, i)),
        PokemonCoreData::kBlockSize);
  }
}

void PokemonDataAccessor::ShuffleBlocks() {
  PokemonCoreData data;
  std::memcpy(&data, pkm_core_data_, sizeof(data));

  for (u32 i = 0; i < PokemonCoreData::kBlockCount; i++) {
    for (u32 j = 0; j < PokemonCoreData::kBlockCount; j++) {
      if (GetBlockPosition(pkm_core_data_->encryption_key, j) == i) {
        std::memcpy(pkm_core_data_->GetBlock(i), data.GetBlock(j),
                    PokemonCoreData::kBlockSize);
        break;
      }
    }
  }
}

void PokemonDataAccessor::Encrypt() {
  ShuffleBlocks();
  ((void (*)(PokemonDataAccessor*))ADDRESS_POKEMON_DATA_ACCESSOR_ENCRYPT)(this);
}

void PokemonDataAccessor::Decrypt() {
  ((void (*)(PokemonDataAccessor*))ADDRESS_POKEMON_DATA_ACCESSOR_DECRYPT)(this);
  UnshuffleBlocks();
}

void PokemonDataAccessor::Initialize(PokemonCoreData* pkm_core_data,
                                     PokemonRuntimeData* pkm_runtime_data) {
  ((void (*)(PokemonDataAccessor*, PokemonCoreData*,
             PokemonRuntimeData*))ADDRESS_POKEMON_DATA_ACCESSOR_INITIALIZE)(
      this, pkm_core_data, pkm_runtime_data);
}