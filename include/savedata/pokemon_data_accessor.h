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

#ifndef SANGO_PLUGIN_POKEMON_DATA_ACCESSOR_H
#define SANGO_PLUGIN_POKEMON_DATA_ACCESSOR_H
#include "common.h"

struct PokemonCoreData;
struct PokemonRuntimeData;

class PokemonDataAccessor {
 public:
  PokemonDataAccessor()
      : vtable(nullptr),
        pkm_runtime_data_(nullptr),
        pkm_core_data_(nullptr),
        is_encrypted_(true) {}

  void Initialize(PokemonCoreData* pkm_core_data,
                  PokemonRuntimeData* pkm_runtime_data);
  void Encrypt();
  void Decrypt();

  PokemonCoreData* GetCoreData() const { return pkm_core_data_; }

  PokemonRuntimeData* GetRuntimeData() const { return pkm_runtime_data_; }

 private:
  void ShuffleBlocks();
  void UnshuffleBlocks();
  u8 GetBlockPosition(u32 encryption_key, u32 index);

  void* vtable;
  PokemonRuntimeData* pkm_runtime_data_;
  PokemonCoreData* pkm_core_data_;
  bool is_encrypted_;
  bool state_[3];
};

#endif  // SANGO_PLUGIN_POKEMON_DATA_ACCESSOR_H
