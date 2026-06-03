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

#include "menu/plugin_menu.h"
#include "ui/color_menu.h"
#include "ui/menu_context.h"

namespace ui {
void LoadPokemonTextureMenu(menu::PluginMenu& menu, void* args) {
}

void LoadLightMenu(menu::PluginMenu& menu, void* args) {
  auto& ctx = *(RendererLightMenuContext*)args;

  menu.Add("Outline Scale", ctx.outline_scale)
      .Add("Outline Color", LoadColorMenu, &ctx.outline_color)
      .Add("Ambient Light Color", LoadColorMenu, &ctx.ambient_color)
      .Add("Diffuse Light Color", LoadColorMenu, &ctx.diffuse_color);
}

void LoadLayoutMenu(menu::PluginMenu& menu, void* args) {
}

void LoadRendererMenu(menu::PluginMenu& menu, void* args) {
  auto& ctx = *(RendererMenuContext*)args;

  menu.Add("Light", LoadLightMenu, &ctx.light)
      .Add("Layout", LoadLayoutMenu)
      .Add("Pokemon Texture", LoadPokemonTextureMenu);
}
} // namespace ui