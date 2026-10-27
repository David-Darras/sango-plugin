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
 * @file page_field_actions.cc
 * @brief The menu pages of the actions of the overworld: the field moves and
 *        the apps.
 */

#include <utility>

#include "core/patch/app_launcher.h"
#include "overworld/patch/field_move.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "overworld/address.h"

namespace ui {
void LoadOverworldFieldMovePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const std::pair<const c8*, FieldMoveId> kMoves[] = {
      {"Cut", FieldMoveId::kCut},
      {"Rock Smash", FieldMoveId::kRockSmash},
      {"Strength", FieldMoveId::kStrength},
      {"Surf", FieldMoveId::kSurf},
#ifndef GAME_XY
      {"Dive", FieldMoveId::kDive},
#endif
      {"Waterfall", FieldMoveId::kWaterfall},
      {"Flash", FieldMoveId::kFlash},
      {"Teleport", FieldMoveId::kTeleport},
      {"Dig", FieldMoveId::kDig},
      {"Sweet Scent", FieldMoveId::kSweetScent},
#ifndef GAME_XY
      {"Secret Power", FieldMoveId::kSecretPower},
#endif
  };

  // Fly opens the Town Map. It does not use the field move menu.
  app.Add("Fly", [](void*) { core::AppLauncher::DoFly(); });
  for (const auto& move : kMoves) {
    app.Add(move.first, [move](void*) {
      overworld::FieldMove::Execute(move.second);
    });
  }
}

static void WonderTrade(void*) {
  static u32 input[] = {0, /*index*/0, /*team*/0xFE, /*team_index*/0};
  ((void(*)(uptr, u32*))overworld::address::kTradePokemon)(0, input);

  MainApplication::GetInstance().ForceClose();
}

void LoadAppPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& launcher = core::AppLauncher::GetInstance();

  // The list does not have the move tutor. It teaches only one move.
  static const std::pair<const c8*, AppId> kApps[] = {
      {"PC", AppId::kBox},
      {"Move Reminder", AppId::kMoveReminder},
      {"Move Deleter", AppId::kMoveDeleter},
  };
  for (const auto& entry : kApps) {
    app.Add(entry.first, [&launcher, entry](void*) {
      launcher.TriggerApp(entry.second);
    });
  }
  app.Add("Wonder Trade", WonderTrade);
}
} // namespace ui
