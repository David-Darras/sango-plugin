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
 * @file screenshot_mode.cc
 * @brief Hides the menu, the overlays and the 2D menus of the game.
 *
 * The declarations are in ui/screenshot_mode.h.
 */

#include "ui/screenshot_mode.h"

#include "renderer/patch/layout_editor.h"
#include "ui/main_application.h"

namespace ui {

void ScreenshotMode::Start() {
  auto& self = GetInstance();
  auto& menu = MainApplication::GetInstance();
  if (menu.IsOpened()) menu.ForceClose();
  self.is_active_ = true;
  renderer::LayoutEditor::SetHideAll(self.hide_game_ui);
}

void ScreenshotMode::Stop() {
  GetInstance().is_active_ = false;
  renderer::LayoutEditor::SetHideAll(false);
}

void ScreenshotMode::Update() {
  auto& self = GetInstance();
  if (!self.is_active_) return;
  // The key of the menu opened it: the mode ends.
  if (MainApplication::GetInstance().IsOpened()) {
    Stop();
    return;
  }
  // The player can change the setting only in the menu, but the layouts of a
  // new screen need the flag again.
  renderer::LayoutEditor::SetHideAll(self.hide_game_ui);
}
} // namespace ui
