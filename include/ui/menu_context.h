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

#ifndef SANGO_PLUGIN_UI_MENU_CONTEXT_H
#define SANGO_PLUGIN_UI_MENU_CONTEXT_H
#include "common.h"

namespace ui {
struct GlobalDataPokemonMenuContext {
  u16 species;
};

struct GlobalDataMoveMenuContext {
  u16 move;
};

struct GlobalDataMenuContext {
  GlobalDataPokemonMenuContext global_data_pokemon;
  GlobalDataMoveMenuContext global_data_move;
};

struct TopMenuContext {
  SINGLETON(TopMenuContext)

public:
  static TopMenuContext& GetInstance() {
    static TopMenuContext instance;
    return instance;
  }

  GlobalDataMenuContext global_data;
};
} // namespace ui

#endif // SANGO_PLUGIN_UI_MENU_CONTEXT_H