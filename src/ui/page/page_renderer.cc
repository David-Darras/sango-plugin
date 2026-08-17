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
#include "feature/feature_h3d_model.h"
#include "feature/feature_text_box.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"

namespace ui {
void LoadPokemonTexturePage(MainApplication& app, void* args) {
  static const char* FILTERS[] = {
      "Normal", // 0
      "Pitch Black", // 1
      "Invert", // 2
      "Darken", // 3
      "Overexposed", // 4
      "Psychedelic", // 5
      "True Saturation", // 6
      "Sepia", // 7
      "Red Tint", // 8
      "Blue Tint", // 9
      "Green Tint", // 10
      "Night Vision", // 11
      "Vintage", // 12
      "Solarize", // 13
      "Chrome Metallic", // 14
      "Gold Metallic", // 15
      "Duotone Ocean Fire", // 16
      "Thermal Cam", // 17
      "Film Noir", // 18
      "Blueprint", // 19
      "X-ray", // 20
      "Toon", // 21
      "Ghost", // 21
  };

  auto& ctx = feature::H3dModel::GetInstance();

  app.Add("Filter", ctx.filter)
     .WithArray(FILTERS, SIZE(FILTERS))
     .Add("Mesh", ctx.mesh);
}

void LoadLightPage(MainApplication& app, void* args) {
  auto& ctx = feature::Light::GetInstance();

  app.Add("Use Outline", ctx.use_outline)
     .Add("Outline Scale", ctx.outline_scale)
     .Add("Outline Color", LoadColorPage, &ctx.outline_color)
     .AddSeparator()
     .Add("Use Ambient Light", ctx.use_ambient_light)
     .Add("Ambient Light Color", LoadColorPage, &ctx.ambient_color)
     .AddSeparator()
     .Add("Use Diffuse Light", ctx.use_diffuse_light)
     .Add("Diffuse Light Color", LoadColorPage, &ctx.diffuse_color);
}

void LoadLayoutTextBoxPage(MainApplication& app, void* args) {
  auto& ctx = feature::TextBox::GetInstance();

  app.Add("Is Enabled", ctx.is_enabled)
     .AddSeparator()
     .Add("Scale X", ctx.scale.x)
     .WithFactor(0.1f)
     .Add("Scale Y", ctx.scale.y)
     .WithFactor(0.1f)
     .AddSeparator()
     .Add("Top Color", LoadColor8Page, &ctx.top_color)
     .Add("Bottom Color", LoadColor8Page, &ctx.bottom_color);
}

void LoadLayoutPicturePage(MainApplication& app, void* args) {
  auto& ctx = feature::Picture::GetInstance();

  app.Add("Is Enabled", ctx.is_enabled)
     .AddSeparator()
     .Add("Scale X", ctx.scale.x)
     .WithFactor(0.1f)
     .Add("Scale Y", ctx.scale.y)
     .WithFactor(0.1f)
     .AddSeparator()
     .Add("Alpha", ctx.alpha)
     .Add("Top Left Color", LoadColor8Page, &ctx.top_left_color)
     .Add("Top Right Color", LoadColor8Page, &ctx.top_right_color)
     .Add("Bottom Left Color", LoadColor8Page, &ctx.bottom_left_color)
     .Add("Bottom Right Color", LoadColor8Page, &ctx.bottom_right_color);
}

void LoadRendererPage(MainApplication& app, void* args) {
  app.Add("Light", LoadLightPage)
     .Add("Layout - Text Box", LoadLayoutTextBoxPage)
     .Add("Layout - Picture", LoadLayoutPicturePage)
     .Add("Pokemon Texture", LoadPokemonTexturePage);
}
} // namespace ui