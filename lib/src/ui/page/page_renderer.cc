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

/**
 * @file page_renderer.cc
 * @brief The menu pages of the Renderer family.
 */

#include "renderer/patch/lighting.h"
#include "renderer/patch/picture_filter.h"
#include "renderer/patch/model_filter.h"
#include "renderer/patch/text_box_filter.h"
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
      "Sepia", // 6
      "Obsidian", // 7
      "Plasma", // 8
      "Sketch", // 9
      "Chrome Metallic", // 10
      "Liquid Chrome", // 11
      "Ghost", // 12
  };



  auto& ctx = renderer::ModelFilter::GetInstance();

  app.Add("Filter", ctx.filter)
     .WithArray(FILTERS, SIZE(FILTERS))
     .WithDescription("A color filter on the Pokemon models in battle.")
     .Add("Mesh", ctx.mesh);
}

void LoadLightPage(MainApplication& app, void* args) {
  auto& ctx = renderer::Lighting::GetInstance();

  app.AddSection("Outlines")
     .Add("Use Outline", ctx.use_outline)
     .WithDescription("Off: the 3D models have no outlines.")
     .Add("Outline Scale", ctx.outline_scale)
     .WithFactor(0.1f)
     .WithDescription("The width of the outlines. 0: the width of the "
                      "game.")
     .Add("Outline Color", LoadColorPage, &ctx.outline_color)
     .AddSection("Ambient Light")
     .Add("Use Ambient Light", ctx.use_ambient_light)
     .Add("Ambient Light Color", LoadColorPage, &ctx.ambient_color)
     .AddSection("Diffuse Light")
     .Add("Use Diffuse Light", ctx.use_diffuse_light)
     .Add("Diffuse Light Color", LoadColorPage, &ctx.diffuse_color);
}

void LoadLayoutTextBoxPage(MainApplication& app, void* args) {
  auto& ctx = renderer::TextBoxFilter::GetInstance();

  app.Add("Is Enabled", ctx.is_enabled)
     .AddSection("Scale")
     .Add("Scale X", ctx.scale.x)
     .WithFactor(0.1f)
     .Add("Scale Y", ctx.scale.y)
     .WithFactor(0.1f)
     .AddSection("Colors")
     .Add("Top Color", LoadColor8Page, &ctx.top_color)
     .Add("Bottom Color", LoadColor8Page, &ctx.bottom_color);
}

void LoadLayoutPicturePage(MainApplication& app, void* args) {
  auto& ctx = renderer::PictureFilter::GetInstance();

  app.Add("Is Enabled", ctx.is_enabled)
     .AddSection("Scale")
     .Add("Scale X", ctx.scale.x)
     .WithFactor(0.1f)
     .Add("Scale Y", ctx.scale.y)
     .WithFactor(0.1f)
     .AddSection("Colors")
     .Add("Alpha", ctx.alpha)
     .Add("Top Left Color", LoadColor8Page, &ctx.top_left_color)
     .Add("Top Right Color", LoadColor8Page, &ctx.top_right_color)
     .Add("Bottom Left Color", LoadColor8Page, &ctx.bottom_left_color)
     .Add("Bottom Right Color", LoadColor8Page, &ctx.bottom_right_color);
}

void LoadRendererPage(MainApplication& app, void* args) {
  app.AddSection("3D Models")
     .Add("Lighting", LoadLightPage)
     .WithDescription("The outlines, the ambient light and the diffuse "
                      "light of the 3D models.")
     .Add("Battle Pokemon Filter", LoadPokemonTexturePage)
     .WithDescription("A color filter on the Pokemon models in battle.")
     .AddSection("2D Layouts")
     .Add("Text Box", LoadLayoutTextBoxPage)
     .WithDescription("The size and the colors of the message boxes.")
     .Add("Picture", LoadLayoutPicturePage)
     .WithDescription("The size and the colors of the pictures of the "
                      "layouts.");
}
} // namespace ui