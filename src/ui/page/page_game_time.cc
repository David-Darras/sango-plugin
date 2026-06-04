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

#include "game/time_manager.h"
#include "ui/main_application.h"

namespace ui {
void LoadGameTimePage(MainApplication& app, void* args) {
  auto& data = game::TimeManager::GetInstance();

  app.Add("Date Time (ms)", *(s64*)ADDRESS_DATE_TIME)
      .AddSeparator()
      .Add("Is Game Time Enabled", data.is_enabled)
      .Add("Last Tick", data.last_tick)
      .Add("First Tick", data.first_tick)
      .Add("Accumulated Seconds", data.accumulated_seconds)
      .Add("Frame Counter", data.frame_counter);
}
} // namespace ui