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
 * @file hack.cc
 * @brief The default entry point: it calls the functions of hack.h.
 */

#include "hack.h"

#include "plugin.h"
#include "ui/page/pages.h"
#include "ui/painter.h"

// The functions of the ROM hack are optional: a weak reference is null when
// the product does not write the function.
__attribute__((weak)) void SetUpHack();
__attribute__((weak)) void LoadHackMenu(ui::MainApplication& app, void* args);
__attribute__((weak)) void UpdateHack();

namespace {
void EveryFrame() {
  if (UpdateHack) UpdateHack();
  plugin::UpdateFrame();
  plugin::DrawFrame();
}
} // namespace

// A product that writes its own Initialize() replaces this function.
__attribute__((weak)) void Initialize() {
  plugin::InitializeEngine();
  if (SetUpHack) SetUpHack();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(),
                   LoadHackMenu ? LoadHackMenu : ui::LoadTopPage);
  plugin::Start(EveryFrame);
}
