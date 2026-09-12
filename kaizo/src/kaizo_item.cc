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

#include "game/constant/item.h"
#include "game/global_data/item.h"

namespace kaizo {
void PatchItemData(global_data::Item* item) {
  switch (static_cast<ItemId>(item->id)) {
    case ItemId::kHpUp:
      item->hp_ev_value = 63;
      break;
    case ItemId::kProtein:
      item->attack_ev_value = 63;
      break;
    case ItemId::kIron:
      item->defense_ev_value = 63;
      break;
    case ItemId::kCarbos:
      item->speed_ev_value = 63;
      break;
    case ItemId::kCalcium:
      item->sp_atk_ev_value = 63;
      break;
    case ItemId::kZinc:
      item->sp_def_ev_value = 63;
      break;
    case ItemId::kMetalCoat:
    case ItemId::kDragonScale:
    case ItemId::kUpGrade:
    case ItemId::kDubiousDisc:
    case ItemId::kDeepSeaTooth:
    case ItemId::kDeepSeaScale:
    case ItemId::kProtector:
    case ItemId::kElectirizer:
    case ItemId::kMagmarizer:
    case ItemId::kReaperCloth:
    case ItemId::kPrismScale:
    case ItemId::kSachet:
    case ItemId::kWhippedDream:
      item->evolve = 1;
      item->use_on_pokemon = true;
      item->field_function = 1;
      break;
  }
}
}