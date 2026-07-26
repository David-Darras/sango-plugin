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
#include "common.h"
#include "game/constant/model.h"

namespace kaizo {
u32 PatchOverworldModels(u32 model, bool is_real_overworld) {
  if (!is_real_overworld) {
    switch (model) {
      case MODEL_ZIGZAGOON:
        return MODEL_GROUDON;
      case MODEL_SKITTY:
        return MODEL_KYOGRE;
    }
  }
  switch (model) {
    case MODEL_BRENDAN:
      return MODEL_STEVEN_STONE;
    case MODEL_SUPPORT_MAY:
      return MODEL_ZINNIA;
    case MODEL_MOM_ORAS:
      return MODEL_MR_STONE;
    case MODEL_SNORLAX_DOLL:
      return MODEL_HOOPAS_RING;
  }
  return model;
}


}