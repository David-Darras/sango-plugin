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

#include "feature/overworld/feature_map_graft.h"
#include "game/constant/map.h"
#include "overlay/pages.h"
#include "overlay/scripts.h"
#include "plugin.h"
#include "ui/painter.h"

static void EveryFrame() {
  plugin::UpdateFrame();
  UpdateFollowingPokemon();
  plugin::DrawFrame();
}

void Initialize() {
  plugin::InitializeEngine();

  // Work in progress: stitching Hoenn towns together.
  feature::MapGraft::Attach(MapId::kLittlerootTown, feature::Side::kUp,
                            MapId::kOldaleTown);
  feature::MapGraft::Link(MapId::kOldaleTown, feature::Side::kUp,
                          MapId::kRustboroCity, 125, 238);
  feature::MapGraft::Link(MapId::kRustboroCity, feature::Side::kDown,
                          MapId::kOldaleTown, 100, 121);
  feature::MapGraft::Link(MapId::kRustboroCity, feature::Side::kRight,
                          MapId::kPetalburgCity, 80, 126);
  feature::MapGraft::Link(MapId::kPetalburgCity, feature::Side::kLeft,
                          MapId::kRustboroCity, 159, 173);
  feature::MapGraft::Link(MapId::kPetalburgCity, feature::Side::kRight,
                          MapId::kFallarborTown, 160, 102);
  feature::MapGraft::Link(MapId::kFallarborTown, feature::Side::kLeft,
                          MapId::kVerdanturfTown, 119, 102);

  script::Install();

  plugin::LoadConfiguration();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), ui::LoadTopPage);
  plugin::Start(EveryFrame);
}
