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
  switch (item->id) {
    case ITEM_HP_UP:
      item->hp_ev_value = 63;
      break;
    case ITEM_PROTEIN:
      item->attack_ev_value = 63;
      break;
    case ITEM_IRON:
      item->defense_ev_value = 63;
      break;
    case ITEM_CARBOS:
      item->speed_ev_value = 63;
      break;
    case ITEM_CALCIUM:
      item->sp_atk_ev_value = 63;
      break;
    case ITEM_ZINC:
      item->sp_def_ev_value = 63;
      break;
    case ITEM_METAL_COAT:
    case ITEM_DRAGON_SCALE:
    case ITEM_UP_GRADE:
    case ITEM_DUBIOUS_DISC:
    case ITEM_DEEP_SEA_TOOTH:
    case ITEM_DEEP_SEA_SCALE:
    case ITEM_PROTECTOR:
    case ITEM_ELECTIRIZER:
    case ITEM_MAGMARIZER:
    case ITEM_REAPER_CLOTH:
    case ITEM_PRISM_SCALE:
    case ITEM_SACHET:
    case ITEM_WHIPPED_DREAM:
      item->evolve = 1;
      item->use_on_pokemon = true;
      item->field_function = 1;
      break;
  }
}
}