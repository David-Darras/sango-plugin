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

#include "core/patch/script_loader.h"
#include "script/patch/native_script.h"
#include "ui/main_application.h"

namespace ui {
void LoadScriptPage(MainApplication& app, void* args) {
  auto& loader = core::ScriptLoader::GetInstance();
  auto& native = script::NativeScript::GetInstance();

  app.Add("Dump Scripts", loader.dump_scripts)
     .Add("Load Edited Scripts", loader.inject_scripts)
     .Add("Log To Screen", loader.log_activity)
     .Add("Scripts Dumped", loader.dumped_count)
     .Add("Scripts Replaced", loader.injected_count)
     .AddSeparator()
     .Add("Skip Key Presses", loader.no_key_press)
     .Add("Skip Cutscenes", loader.no_cutscene)
     .AddSeparator()
     .Add("Log C++ Scripts", native.log_activity)
     .Add("C++ Scripts Run", native.run_count);
}
} // namespace ui
