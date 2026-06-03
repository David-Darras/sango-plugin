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
  u16 species = 0;
};

struct GlobalDataMoveMenuContext {
  u16 move = 0;
};

struct GlobalDataMenuContext {
  GlobalDataPokemonMenuContext global_data_pokemon;
  GlobalDataMoveMenuContext global_data_move;
};

struct RendererLightMenuContext {
  f32 outline_scale = 1.0f;
  Color outline_color = Color{0, 0, 0, 1};
  Color ambient_color = Color{1, 1, 1, 1};
  Color diffuse_color = Color{1, 1, 1, 1};
};

struct RendererMenuContext {
  RendererLightMenuContext light;
};

struct TopMenuContext {
  SINGLETON(TopMenuContext)

public:
  STATIC_INLINE TopMenuContext& GetInstance() {
    static TopMenuContext instance;
    return instance;
  }

  GlobalDataMenuContext global_data;
  RendererMenuContext renderer;
};
} // namespace ui

#endif // SANGO_PLUGIN_UI_MENU_CONTEXT_H