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
#include "ui/main_application.h"

namespace ui {
void LoadAppPage(MainApplication& app, void* args) {
  auto& ctx = feature::GameApp::GetInstance();

  const std::pair<const char*, u32> apps[] = {
      // {"Tutor", 7},
      {"PC", 17},
      {"Remind", 9},
      {"Delete", 8}
  };

  for (const auto& app_pair : apps) {
    app.Add(app_pair.first, [&ctx, app_pair](void*) {
      ctx.TriggerApp(app_pair.second);
    });
  }
}
} // namespace ui