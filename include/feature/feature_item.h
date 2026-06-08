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
#include "hook_manager.h"
#include "game/constant/item.h"
#include "game/global_data/item.h"

#define ADDRESS_LOAD_ITEM_DATA (0x003B7B9C)

namespace feature {
class Item {
  MAKE_SINGLETON(Item)
  STATIC_INLINE void Initialize() {
    // No limit to 100 for EVs with ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, etc.
    WRITE(vu32, 0x0047888C, 0xE35000FF);
    WRITE(vu32, 0x004788F0, 0xE35000FF);
    WRITE(vu32, 0x004788FC, 0x03A000FF);
    WRITE(vu32, 0x0046E104, 0xE35B00FF);
    WRITE(vu32, 0x0046E1A4, 0xE35B00FF);
    WRITE(vu32, 0x0046E244, 0xE35800FF);
    WRITE(vu32, 0x0046E2E0, 0xE35900FF);
    WRITE(vu32, 0x0046E37C, 0xE35A00FF);
    WRITE(vu32, 0x0046E41C, 0xE35A00FF);

    // No level limit with rare candy : WRITE(vu32, 0x0046DED8, 0xE35000FF);
    HookManager::Initialize(HookID::kLoadItemData, ADDRESS_LOAD_ITEM_DATA,
                            (uptr)LoadItemDataHook);
  }

  static uptr LoadItemDataHook(global_data::Item* item, u32 id, void* heap) {
    uptr res = HookManager::Call<uptr>(HookID::kLoadItemData, item, id, heap);
    if (id == ITEM_PP_UP) {
      item->hp_ev_value = 4;
    } else if (id == ITEM_PROTEIN) {
      item->attack_ev_value = 4;
    } else if (id == ITEM_IRON) {
      item->defense_ev_value = 4;
    } else if (id == ITEM_CARBOS) {
      item->speed_ev_value = 4;
    } else if (id == ITEM_CALCIUM) {
      item->sp_atk_ev_value = 4;
    } else if (id == ITEM_ZINC) {
      item->sp_def_ev_value = 4;
    }
    return res;
  }
};
}