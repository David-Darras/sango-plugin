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
 * @file page_data_scripts.cc
 * @brief The menu pages of the scripts without code: the list of the scripts,
 *        then the steps of one script.
 */

#include <cstdio>

#include "overworld/patch/trigger_editor.h"
#include "script/patch/data_scripts.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using script::DataScripts;

constexpr u32 kLabelLength = 56;
c8 script_labels[DataScripts::kMaxScripts][kLabelLength];
c8 script_note[kLabelLength];

u32 selected_script = 0;

const c8* kActionNames[] = {"End",        "Say",        "Ask Yes/No",
                            "Give Item",  "Add Money",  "Set Flag",
                            "Reset Flag", "Require Flag", "Play Sound",
                            "Wait Frames", "Run Game Script", "Give Pokemon"};

void* ToArgument(u32 slot) {
  return reinterpret_cast<void*>(static_cast<uptr>(slot + 1));
}

void SaveScripts(void*) {
  MainApplication::GetInstance().ShowToast(
      DataScripts::Save() ? "Saved to the SD card." : "The SD card refused.");
}

void ClearScript(void*) {
  for (u32 i = 0; i < DataScripts::kMaxSteps; i++) {
    *DataScripts::GetStep(selected_script, i) = DataScripts::Step();
  }
  MainApplication::GetInstance().Refresh();
}

void AddSignWithScript(void*) {
  overworld::SignEvent* sign = overworld::TriggerEditor::AddSign();
  if (sign == nullptr) {
    MainApplication::GetInstance().ShowToast("No room, or no map events.");
    return;
  }
  sign->id = static_cast<u16>(DataScripts::kFirstScript + selected_script);
  MainApplication::GetInstance().ShowToast("Sign added on your tile.");
}

void AddTriggerWithScript(void*) {
  overworld::TriggerEvent* trigger = overworld::TriggerEditor::AddTrigger();
  if (trigger == nullptr) {
    MainApplication::GetInstance().ShowToast("No room, or no map events.");
    return;
  }
  trigger->id = static_cast<u16>(DataScripts::kFirstScript + selected_script);
  MainApplication::GetInstance().ShowToast("Trigger added on your tile.");
}
} // namespace

void LoadDataScriptsPage(MainApplication& app, void* args) {
  app.AddSection("Scripts Without Code");
  snprintf(script_note, kLabelLength, "Numbers %lu to %lu",
           (unsigned long)DataScripts::kFirstScript,
           (unsigned long)(DataScripts::kFirstScript + DataScripts::kMaxScripts - 1));
  app.Add(script_note)
     .WithDescription("A sign, a trigger or a character that uses one of these "
                      "numbers starts the script. A script is a list of steps.");
  for (u32 s = 0; s < DataScripts::kMaxScripts; s++) {
    const DataScripts::Step* first = DataScripts::GetStep(s, 0);
    snprintf(script_labels[s], kLabelLength, "Script %lu  %s",
             (unsigned long)(DataScripts::kFirstScript + s),
             DataScripts::IsUsed(s) ? kActionNames[first->action] : "(empty)");
    app.Add(script_labels[s], LoadDataScriptPage, ToArgument(s))
       .WithDescription("Opens the steps of this script.");
  }
  app.AddSection("File");
  app.Add("Save To The SD Card", SaveScripts)
     .WithDescription("Writes sdmc:/sango/scripts.txt. The game reads it when "
                      "it starts.");
}

void LoadDataScriptPage(MainApplication& app, void* args) {
  if (args != nullptr) {
    selected_script = static_cast<u32>(reinterpret_cast<uptr>(args)) - 1;
  }
  if (selected_script >= DataScripts::kMaxScripts) return;
  for (u32 i = 0; i < DataScripts::kMaxSteps; i++) {
    DataScripts::Step* step = DataScripts::GetStep(selected_script, i);
    snprintf(script_labels[i], kLabelLength, "Step %lu", (unsigned long)(i + 1));
    app.AddSection(script_labels[i])
       .Add("Action", step->action)
       .WithArray(kActionNames, SIZE(kActionNames))
       .WithBounds(0, SIZE(kActionNames) - 1)
       .WithDescription("End stops the script. The steps after an End do not "
                        "run.")
       .Add("Parameter 1", step->parameter_1)
       .WithDescription("Say: 0 sign window, 1 talk window. Give Item: item "
                        "number. Add Money: amount. Flags: flag number. "
                        "Play Sound: sound number. Wait: frames. Run Game "
                        "Script: script number. Give Pokemon: gift number.")
       .Add("Parameter 2", step->parameter_2)
       .WithDescription("Give Item: how many (0 is 1). Add Money: 1 takes the "
                        "money.")
       .Add("Text (1)", step->text, DataScripts::kPartLength)
       .WithDescription("The text of a Say step, with markers like [red] and "
                        "[/color]. See the game text editor.")
       .Add("Text (2)", step->text_2, DataScripts::kPartLength)
       .WithDescription("The rest of the text: the keyboard holds 31 "
                        "characters.");
  }
  app.AddSection("Use");
  app.Add("Add A Sign Here", AddSignWithScript)
     .WithDescription("Adds a sign on your tile that starts this script.")
     .Add("Add A Trigger Here", AddTriggerWithScript)
     .WithDescription("Adds a trigger on your tile that starts this script "
                      "when you walk in.")
     .Add("Save To The SD Card", SaveScripts)
     .Add("Clear This Script", ClearScript)
     .WithConfirm();
}

} // namespace ui
