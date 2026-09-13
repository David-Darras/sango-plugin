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
 * The overlay: every page of the library under one menu, for exploring and
 * editing the game while it runs.
 */

#include "overworld/patch/map_graft.h"
#include "overworld/constant/map.h"
#include "overlay/scripts.h"
#include "plugin.h"
#include "core/patch/archive.h"
#include "ui/page/pages.h"
#include "ui/painter.h"

static void EveryFrame() {
  plugin::UpdateFrame();
  UpdateFollowingPokemon();
  plugin::DrawFrame();
}

void Initialize() {
  plugin::InitializeEngine();

  // Work in progress: stitching Hoenn towns together.
  // overworld::MapGraft::Attach(MapId::kLittlerootTown, overworld::Facing::kUp,
  //                           MapId::kOldaleTown);
  // overworld::MapGraft::Link(MapId::kOldaleTown, overworld::Facing::kUp,
  //                         MapId::kRustboroCity, 125, 238);
  // overworld::MapGraft::Link(MapId::kRustboroCity, overworld::Facing::kDown,
  //                         MapId::kOldaleTown, 100, 121);
  // overworld::MapGraft::Link(MapId::kRustboroCity, overworld::Facing::kRight,
  //                         MapId::kPetalburgCity, 80, 126);
  // overworld::MapGraft::Link(MapId::kPetalburgCity, overworld::Facing::kLeft,
  //                         MapId::kRustboroCity, 159, 173);
  // overworld::MapGraft::Link(MapId::kPetalburgCity, overworld::Facing::kRight,
  //                         MapId::kFallarborTown, 160, 102);
  // overworld::MapGraft::Link(MapId::kFallarborTown, overworld::Facing::kLeft,
  //                         MapId::kVerdanturfTown, 119, 102);

  script::Install();

  plugin::LoadConfiguration();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), ui::LoadTopPage);
  plugin::Start(EveryFrame);
}
