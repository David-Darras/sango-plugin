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

#include "core/patch/app_launcher.h"
#include "battle/patch/setup.h"
#include "core/patch/game_speed.h"
#include "overworld/patch/camera.h"
#include "overworld/patch/field_move.h"
#include "overworld/patch/field.h"
#include "overworld/native/encounter_state.h"
#include "savedata/native/event_table.h"
#include "savedata/native/misc.h"
#include "savedata/native/savedata.h"
#include "system/native/sound.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "kaizo/kaizo.h"

namespace kaizo {
using ui::MainApplication;

static void LoadHmPage(MainApplication& app, void* args) {
  auto& misc = savedata::Misc::GetInstance();
  auto badge_count = misc.GetBadgesCount();
  if (badge_count == 0) {
    app.Add("(NONE)");
    return;
  }
  if (badge_count >= 1)
    app.Add("Cut", [&](void*) {
      overworld::FieldMove::Execute(FieldMoveId::kCut);
    });
  if (badge_count >= 2)
    app.Add("Rock Smash", [&](void*) {
      overworld::FieldMove::Execute(FieldMoveId::kRockSmash);
    });
  if (badge_count >= 3)
    app.Add("Strength", [&](void*) {
      overworld::FieldMove::Execute(FieldMoveId::kStrength);
    });
  if (badge_count >= 4)
    app.Add(
        "Fly", [&](void*) { core::AppLauncher::DoFly(); });
  if (badge_count >= 5)
    app.Add("Surf", [&](void*) {
      overworld::FieldMove::Execute(FieldMoveId::kSurf);
    });
  if (badge_count >= 6)
    app.Add("Dive", [&](void*) {
      overworld::FieldMove::Execute(FieldMoveId::kDive);
    });
  if (badge_count >= 7)
    app.Add("Waterfall", [&](void*) {
      overworld::FieldMove::Execute(FieldMoveId::kWaterfall);
    });
}

void LoadMenuPage(MainApplication& app, void* args) {
  // Kaizo mode only exposes this single page (no access to the full
  // Overworld/Battle menu tree), so it keeps its own direct camera state
  // control rather than pointing to the dedicated Overworld > Camera page.
  static const c8* CAMERA_STATES[] = {
      "Idle", "Tps", "Rotate", "Top", "Fpv", "Free"
  };

  auto& bgm = overworld::Field::GetInstance().background_music;
  auto& camera = overworld::Camera::GetInstance();
  auto& speed = core::GameSpeed::GetInstance().game_speed;

  app.Add("Camera", camera.overworld_state)
     .WithArray(CAMERA_STATES, SIZE(CAMERA_STATES))
     .Add("Speed", speed)
     .WithBounds(1, 2)
     .Add("Radio", bgm)
     .WithCallback([&](void*) { sys::Sound::PlayBackgroundMusic(bgm); })
     .WithBounds(0, static_cast<u32>(BackgroundMusicId::kCount) - 1)
     .Add("HM", LoadHmPage)
     .Add("App", ui::LoadAppPage)
     .Add("Repel", CheatCodeId::kNoEncounter);
}
} // namespace kaizo