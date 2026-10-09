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
 * @file page_script.cc
 * @brief The menu pages of the Scripts family.
 */

#include "core/patch/script_loader.h"
#include "script/patch/native_script.h"
#include "ui/main_application.h"

namespace ui {
void LoadScriptPage(MainApplication& app, void* args) {
  auto& loader = core::ScriptLoader::GetInstance();
  auto& native = script::NativeScript::GetInstance();

  app.AddSection("Overworld Scripts")
     .Add("Dump Scripts", loader.dump_scripts)
     .WithDescription("Saves each script that the game loads to the SD "
                      "card.")
     .Add("Load Edited Scripts", loader.inject_scripts)
     .WithDescription("Loads the changed scripts from the SD card.")
     .Add("Log To Screen", loader.log_activity)
     .WithDescription("Writes the name of each loaded script to the log.")
     .Add("Scripts Dumped", loader.dumped_count)
     .WithReadOnly()
     .Add("Scripts Replaced", loader.injected_count)
     .WithReadOnly()
     .AddSection("Skip")
     .Add("Skip Key Presses", loader.no_key_press)
     .WithDescription("The messages continue without a key press.")
     .Add("Skip Cutscenes", loader.no_cutscene)
     .AddSection("C++ Scripts")
     .Add("Log C++ Scripts", native.log_activity)
     .WithDescription("Writes the start and the end of each C++ script to "
                      "the log.")
     .Add("C++ Scripts Run", native.run_count)
     .WithReadOnly();
}
} // namespace ui
