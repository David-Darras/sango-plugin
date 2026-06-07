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
#include "feature/feature_field_move.h"
#include "game/overworld/encounter.h"
#include "game/savedata/event_table.h"
#include "game/savedata/savedata.h"
#include "ui/main_application.h"
#include "ui/page/page_top.h"

namespace ui {
void LoadHmPage(MainApplication& app, void* args) {
  app.Add("Cut", [&](void*) { feature::FieldMove::Execute(0); })
     .Add("Rock Smash", [&](void*) { feature::FieldMove::Execute(4); })
     .Add("Strength", [&](void*) { feature::FieldMove::Execute(3); })
     .Add("Fly", [&](void*) { feature::AppHookContext::DoFly(); })
     .Add("Surf", [&](void*) { feature::FieldMove::Execute(1); })
     .Add("Dive", [&](void*) { feature::FieldMove::Execute(10); })
     .Add("Waterfall", [&](void*) { feature::FieldMove::Execute(2); });
}

void LoadNuzlockePage(MainApplication& app, void* args) {
  app.Add("Reset Tr", [](void*) {
       auto& event_table = savedata::EventTable::GetInstance();
       for (u32 i = 1740; i < 2719; i++) {
         event_table.Reset(i);
       }
     })
     .Add("HM", LoadHmPage)
     .Add("App", LoadAppPage)
     .Add("Repel", CheatCodeId::kNoEncounter);
}
} // namespace ui