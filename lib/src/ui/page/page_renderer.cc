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
#include "renderer/patch/model_filter.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"
#include "ui/free_camera_application.h"
#include "ui/screenshot_mode.h"

namespace ui {
namespace {
// The color filters of the Pokemon in battle.
const char* FILTERS[] = {
    "RGB",
    "GRB",
    "BGR",
    "RBG",
    "GBR",
    "BRG",
    "Inverted RGB",
    "Inverted GRB",
    "Inverted BGR",
    "Darker",
    "Brighter",
    "All Black",
    "All White",
    "Gray",
    "Sepia",
    "Light Only",
    "Specular Only",
    "Rim Light",
    "Liquid Metal",
    "Ghost",
};
} // namespace

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

void LoadScreenshotPage(MainApplication& app, void* args) {
  auto& mode = ScreenshotMode::GetInstance();

  app.AddSection("Options")
     .Add("Hide The Game Menus", mode.hide_game_ui)
     .WithDescription("Hides the 2D menus of the game: the HUD, the text "
                      "boxes and the buttons.")
     .Add("Hide The Overlays", mode.hide_overlays)
     .WithDescription("Hides the overlays of the plugin: performance, "
                      "world, type helper.")
     .Add("Freeze The Game", mode.freeze_game)
     .WithDescription("Stops the game. Some effects of the free camera "
                      "need the game to run: turn it off then.")
     .AddSection("Start")
     .Add("Start The Screenshot Mode", [](void*) { ScreenshotMode::Start(); })
     .WithDescription("Closes the menu. Use the free camera or walk mode "
                      "(L + R) as usual. The menu button stops the mode and "
                      "opens the menu again.")
     .Add("Start With The Free Camera", [](void*) {
       ScreenshotMode::Start();
       FreeCameraApplication::Open();
     })
     .WithDescription("The same, and the free camera starts at once. The "
                      "Start button leaves the camera, and the mode goes "
                      "on.");
}

void LoadGraphicsPage(MainApplication& app, void* args) {
  app.AddSection("3D Models")
     .Add("Lighting", LoadLightPage)
     .WithDescription("The outlines, the ambient light and the diffuse "
                      "light of the 3D models.")
     .Add("Pokemon Filter", renderer::ModelFilter::GetInstance().filter)
     .WithArray(FILTERS, SIZE(FILTERS))
     .WithDescription("A color filter on all the 3D Pokemon: in battle, and in "
                      "the party, the PC, the Pokedex and the other menus.")
     .Add("Model Viewer", LoadModelViewerPage)
     .WithDescription("Lists the 3D models of the screen. Hide their parts "
                      "and change the colors of their materials.")
     .Add("Model Loader (Unstable)", LoadModelLoaderPage)
     .WithDescription("Shows a model of a character or a Pokemon near the "
                      "player.")
     .Add("Screenshot Mode", LoadScreenshotPage)
     .WithDescription("Hides the menu, the overlays and the menus of the "
                      "game, to take a clean screenshot.")
     .AddSection("2D Layouts")
     .Add("Layouts", LoadLayoutsPage)
     .WithDescription("The panes of the menus of the game, as a tree: move, "
                      "hide, resize and color them. The changes can be "
                      "saved.");
}
} // namespace ui