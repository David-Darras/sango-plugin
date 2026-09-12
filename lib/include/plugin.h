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

#pragma once

#include "common.h"

namespace ui {
class Painter;
class MainApplication;
}

/**
 * What every product built on the library does the same way. A product's
 * `Initialize()` (called from main) typically is:
 *
 *   plugin::InitializeEngine();      // hooks every feature
 *   ... configure features, install callbacks, register content ...
 *   plugin::OpenMenu(painter, root); // the in-game menu
 *   plugin::Start(EveryFrame);       // hook the game loop
 *
 * and its `EveryFrame()` calls plugin::UpdateFrame() and plugin::DrawFrame()
 * around whatever it needs to do itself.
 */
namespace plugin {
typedef void (*PageLoader)(ui::MainApplication& app, void* args);

/// Mounts the SD card and installs every feature of the library. Features
/// stay passive until a product configures them.
void InitializeEngine();

/// Loads the saved configuration (feature settings and theme) from the SD.
void LoadConfiguration();

/// Sets the menu up with the product's look and root page.
void OpenMenu(ui::Painter& painter, PageLoader root_page);

/// Hooks the game loop; `every_frame` runs once per frame from then on.
void Start(void (*every_frame)());

/// The per-frame work shared by every product: menu input, cheat codes and
/// the features that poll the field. Call before drawing.
void UpdateFrame();

/// Draws the menu and the overlays on both screens.
void DrawFrame();
} // namespace plugin
