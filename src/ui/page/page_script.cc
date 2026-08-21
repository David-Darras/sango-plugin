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

#include "feature/feature_script.h"
#include "ui/main_application.h"

namespace ui {
void LoadScriptPage(MainApplication& app, void* args) {
  auto& ctx = feature::Script::GetInstance();

  app.Add("Dump Scripts", ctx.dump_scripts)
     .Add("Load Edited Scripts", ctx.inject_scripts)
     .Add("Log To Screen", ctx.log_activity)
     .AddSeparator()
     .Add("Scripts Dumped", ctx.dumped_count)
     .Add("Scripts Replaced", ctx.injected_count);
}
} // namespace ui
