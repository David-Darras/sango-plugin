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

#include "feature/feature_app.h"
#include "feature/feature_battle_config.h"
#include "feature/feature_engine.h"
#include "feature/feature_field_move.h"
#include "feature/feature_overworld.h"
#include "game/overworld/encounter.h"
#include "game/savedata/event_table.h"
#include "game/savedata/savedata.h"
#include "system/sound.h"
#include "ui/main_application.h"
#include "ui/page/page_top.h"

namespace ui {
void LoadHmPage(MainApplication& app, void* args) {
  auto& misc = savedata::Misc::GetInstance();
  auto badge_count = misc.GetBadgesCount();
  if (badge_count == 0) {
    app.Add("(NONE)");
    return;
  }
  if (badge_count >= 1)
    app.Add("Cut", [&](void*) {
      feature::FieldMove::Execute(0);
    });
  if (badge_count >= 2)
    app.Add("Rock Smash", [&](void*) {
      feature::FieldMove::Execute(4);
    });
  if (badge_count >= 3)
    app.Add("Strength", [&](void*) {
      feature::FieldMove::Execute(3);
    });
  if (badge_count >= 4)
    app.Add(
        "Fly", [&](void*) { feature::GameApp::DoFly(); });
  if (badge_count >= 5)
    app.Add("Surf", [&](void*) {
      feature::FieldMove::Execute(1);
    });
  if (badge_count >= 6)
    app.Add("Dive", [&](void*) {
      feature::FieldMove::Execute(10);
    });
  if (badge_count >= 7)
    app.Add("Waterfall", [&](void*) {
      feature::FieldMove::Execute(2);
    });
}

void LoadKaizoPage(MainApplication& app, void* args) {
  auto& bgm = feature::Overworld::GetInstance().background_music;
  auto& camera = feature::Camera::GetInstance();
  auto& speed = feature::Engine::GetInstance().game_speed;

  app.Add("Camera", camera.state)
     .WithBounds(0, 4)
     .Add("Speed", speed)
     .WithBounds(1, 2)
     .Add("Radio", bgm)
     .WithCallback([&](void*) { Sound::PlayBackgroundMusic(bgm); })
     .WithBounds(0, 250)
     .Add("HM", LoadHmPage)
     .Add("App", LoadAppPage)
     .Add("Repel", CheatCodeId::kNoEncounter);
}
} // namespace ui