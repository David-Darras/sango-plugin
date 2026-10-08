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
 * @file data_accessor.h
 * @brief Encrypts and decrypts the data of a Pokémon.
 */

#pragma once
#include <cstring>

#include "common.h"
#include "pokemon/native/core_data.h"

namespace pokemon {
struct RuntimeData;

/**
 * @brief Encrypts and decrypts the CoreData of a Pokémon.
 *
 * @code
 * pokemon->accessor->Decrypt();
 * pokemon->core->moves[0] = MoveId::kSurf;
 * pokemon->accessor->Encrypt();
 * @endcode
 */
class DataAccessor {
public:
  DataAccessor()
    : vtable(nullptr),
      pkm_runtime_data_(nullptr),
      pkm_core_data_(nullptr),
      is_encrypted_(true) {
  }

  /// Links the accessor to the data of a Pokémon.
  void Initialize(CoreData* pkm_core_data,
                  RuntimeData* pkm_runtime_data) {
    ((void (*)(DataAccessor*, CoreData*,
               RuntimeData*))address::kDataAccessorInitialize)(
        this, pkm_core_data, pkm_runtime_data);
  }

  /// Encrypts the data. Call it after each change.
  void Encrypt() {
    ShuffleBlocks();
    ((void (*)(DataAccessor*))
      address::kDataAccessorEncrypt)(this);
  }

  /// Decrypts the data. Call it before you read or change the data.
  void Decrypt() {
    ((void (*)(DataAccessor*))
      address::kDataAccessorDecrypt)(this);
    UnshuffleBlocks();
  }

  /// Returns the saved data.
  CoreData* GetCoreData() const { return pkm_core_data_; }

  /// Returns the calculated data (level, HP, stats).
  RuntimeData* GetRuntimeData() const { return pkm_runtime_data_; }

  /// Returns true when the data is encrypted.
  bool IsEncrypted() const { return is_encrypted_; }

private:
  void ShuffleBlocks() {
    CoreData data;
    std::memcpy(&data, pkm_core_data_, sizeof(data));

    for (u32 i = 0; i < CoreData::kBlockCount; i++) {
      for (u32 j = 0; j < CoreData::kBlockCount; j++) {
        if (GetBlockPosition(pkm_core_data_->encryption_key, j) == i) {
          std::memcpy(pkm_core_data_->GetBlock(i), data.GetBlock(j),
                      CoreData::kBlockSize);
          break;
        }
      }
    }
  }

  void UnshuffleBlocks() {
    CoreData data;
    std::memcpy(&data, pkm_core_data_, sizeof(data));

    for (u32 i = 0; i < CoreData::kBlockCount; i++) {
      std::memcpy(
          pkm_core_data_->GetBlock(i),
          data.GetBlock(GetBlockPosition(pkm_core_data_->encryption_key, i)),
          CoreData::kBlockSize);
    }
  }

  static u8 GetBlockPosition(u32 encryption_key, u32 index) {
    static const u8 block_positions[32][4] = {
        {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 3, 1, 2}, {0, 2, 3, 1},
        {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2}, {2, 0, 1, 3}, {3, 0, 1, 2},
        {2, 0, 3, 1}, {3, 0, 2, 1}, {1, 2, 0, 3}, {1, 3, 0, 2}, {2, 1, 0, 3},
        {3, 1, 0, 2}, {2, 3, 0, 1}, {3, 2, 0, 1}, {1, 2, 3, 0}, {1, 3, 2, 0},
        {2, 1, 3, 0}, {3, 1, 2, 0}, {2, 3, 1, 0}, {3, 2, 1, 0}, {0, 1, 2, 3},
        {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 3, 1, 2}, {0, 2, 3, 1}, {0, 3, 2, 1},
        {1, 0, 2, 3}, {1, 0, 3, 2},
    };
    return block_positions[(encryption_key & (0b11111 << 13)) >> 13][index];
  }

  void* vtable;
  RuntimeData* pkm_runtime_data_;
  CoreData* pkm_core_data_;
  bool is_encrypted_;
  bool state_[3];
};

} // namespace pokemon
