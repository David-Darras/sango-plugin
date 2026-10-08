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
 * @file plugin.h
 * @brief The functions that a product calls to start the plugin.
 *
 * A product calls them in this order, in its Initialize() function:
 * 1. InitializeEngine(): installs the features of the library.
 * 2. (The product sets its settings and its callbacks.)
 * 3. OpenMenu(): prepares the menu.
 * 4. Start(): runs a function of the product at each frame.
 *
 * The function of each frame calls UpdateFrame(), then DrawFrame().
 *
 * @see docs/concepts/architecture.md
 */

#pragma once

#include "common.h"

namespace ui {
class Painter;
class MainApplication;
}

namespace plugin {
/// A menu page: the first page of the menu.
typedef void (*PageLoader)(ui::MainApplication& app, void* args);

/// Installs the hooks and the patches of all the features of the library.
void InitializeEngine();

/**
 * @brief Prepares the menu of the plugin.
 * @param painter The object that draws the menu (for example
 *        ui::MainAppPainter::GetInstance()).
 * @param root_page The first page of the menu.
 */
void OpenMenu(ui::Painter& painter, PageLoader root_page);

/**
 * @brief Starts the plugin.
 * @param every_frame A function that the game calls one time for each frame.
 *        It must call UpdateFrame() and DrawFrame().
 */
void Start(void (*every_frame)());

/// Updates the menu, the cheat codes and the features that change at each
/// frame. Call it one time for each frame.
void UpdateFrame();

/// Draws the menu on the two screens. Call it one time for each frame,
/// after UpdateFrame().
void DrawFrame();
} // namespace plugin
