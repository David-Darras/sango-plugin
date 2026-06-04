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

#include "feature/feature_light.h"
#include "feature/feature_picture.h"
#include "feature/feature_pokemon_texture.h"
#include "feature/feature_text_box.h"
#include "../../../include/ui/plugin_menu.h"
#include "ui/common_menu.h"

namespace ui {
void LoadPokemonTextureMenu(PluginMenu& menu, void* args) {
  static const char* FILTERS[] = {
      "RGB",
      "BGR",
      "GRB",
      "RBG",
      "BRG",
      "GBR",
      "Saturate",
      "Grayscale",
      "Invert",
      "Warm",
      "Cool",
      "Threshold",
      "Sepia",
      "Fill"
  };

  auto& ctx = feature::PokemonTextureHookContext::GetInstance();

  menu.Add("Filter", ctx.filter)
      .WithArray(FILTERS, SIZE(FILTERS))
      .Add("Fill - Red", ctx.red)
      .Add("Fill - Green", ctx.green)
      .Add("Fill - Blue", ctx.blue);
}

void LoadLightMenu(PluginMenu& menu, void* args) {
  auto& ctx = feature::LightHookContext::GetInstance();

  menu.Add("Use Outline", ctx.use_outline)
      .Add("Outline Scale", ctx.outline_scale)
      .Add("Outline Color", LoadColorMenu, &ctx.outline_color)
      .AddSeparator()
      .Add("Use Ambient Light", ctx.use_ambient_light)
      .Add("Ambient Light Color", LoadColorMenu, &ctx.ambient_color)
      .AddSeparator()
      .Add("Use Diffuse Light", ctx.use_diffuse_light)
      .Add("Diffuse Light Color", LoadColorMenu, &ctx.diffuse_color);
}

void LoadLayoutTextBoxMenu(PluginMenu& menu, void* args) {
  auto& ctx = feature::TextBoxHookContext::GetInstance();

  menu.Add("Is Enabled", ctx.is_enabled)
      .AddSeparator()
      .Add("Scale X", ctx.scale.x)
      .WithFactor(0.1f)
      .Add("Scale Y", ctx.scale.y)
      .WithFactor(0.1f)
      .AddSeparator()
      .Add("Top Color", LoadColor8Menu, &ctx.top_color)
      .Add("Bottom Color", LoadColor8Menu, &ctx.bottom_color);
}

void LoadLayoutPictureMenu(PluginMenu& menu, void* args) {
  auto& ctx = feature::PictureHookContext::GetInstance();

  menu.Add("Is Enabled", ctx.is_enabled)
      .AddSeparator()
      .Add("Scale X", ctx.scale.x)
      .WithFactor(0.1f)
      .Add("Scale Y", ctx.scale.y)
      .WithFactor(0.1f)
      .AddSeparator()
      .Add("Alpha", ctx.alpha)
      .Add("Top Left Color", LoadColor8Menu, &ctx.top_left_color)
      .Add("Top Right Color", LoadColor8Menu, &ctx.top_right_color)
      .Add("Bottom Left Color", LoadColor8Menu, &ctx.bottom_left_color)
      .Add("Bottom Right Color", LoadColor8Menu, &ctx.bottom_right_color);
}

void LoadRendererMenu(PluginMenu& menu, void* args) {
  menu.Add("Light", LoadLightMenu)
      .Add("Layout - Text Box", LoadLayoutTextBoxMenu)
      .Add("Layout - Picture", LoadLayoutPictureMenu)
      .Add("Pokemon Texture", LoadPokemonTextureMenu);
}
} // namespace ui