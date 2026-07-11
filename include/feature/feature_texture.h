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
#include <cstring>

#include "hook_manager.h"
#include "ui/log_application.h"

namespace feature {
class PokemonIconTexture {
  MAKE_SINGLETON(PokemonIconTexture)
  void* addr = nullptr;
  u32 size = 0;

public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadPokemonIconTexture, 0x00354EEC,
                            (uptr)LoadPokemonIconTexture);
  }

  static void LoadPokemonIconTexture(uptr sys, void* heap, u32 texture_id,
                                     u32 archive_id, u32 data_id,
                                     bool compressed) {
    HookManager::Call<void>(HookID::kLoadPokemonIconTexture, sys, heap,
                            texture_id, archive_id, data_id, compressed);
    void* buffer = (void*)*(u32*)(
      *(u32*)(*(u32*)(sys + 4) + 4 * texture_id) + 0x20);
    u32 size = *(u32*)(*(u32*)(*(u32*)(sys + 4) + 4 * texture_id) + 0x30);
    // ui::LogApplication::Print(u"%u,%u,%u,%u,%p,%u", texture_id, archive_id,
    //                           data_id,
    //                           compressed, buffer, size);
    GetInstance().addr = buffer;
    GetInstance().size = size;
  }
};
}