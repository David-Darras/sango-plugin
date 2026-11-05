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
 * @file page_maker.cc
 * @brief The Sango Maker page: reads the text files again, shows their
 * problems, and changes the flags and the variables of the ROM hack.
 */

#include "core/data_export.h"
#include "core/data_files.h"
#include "pokemon/patch/custom_shop.h"
#include "savedata/patch/hack_save.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
namespace {

using core::DataExport;
using core::DataFiles;
using savedata::HackSave;

// The species of the entry "Export The Species".
u16 export_species = 1;

void ShowExportResult(bool is_written, const c8* file) {
  MainApplication::GetInstance().ShowToast(is_written ? file
                                                      : "Nothing to export");
}

// The flag and the variable that the page shows, and their values.
u16 flag_number = 0;
bool flag_value = false;
u16 variable_number = 0;
u16 variable_value = 0;

void ReadValues(void*) {
  flag_value = HackSave::GetFlag(flag_number);
  variable_value = HackSave::GetVariable(variable_number);
}

void ReadFilesAgain(void*) {
  auto& app = MainApplication::GetInstance();
  if (DataFiles::Reload()) {
    app.ShowToast(DataFiles::GetProblemCount() == 0
                      ? "Files read again"
                      : "Files read again, with problems");
  } else {
    app.ShowToast("A script runs: the files are read at its end");
  }
  app.Refresh();
}

} // namespace

void LoadMakerPage(MainApplication& app, void* args) {
  auto& files = DataFiles::GetInstance();
  ReadValues(nullptr);
  app.AddSection("Files")
     .Add("Edit The Files", LoadMakerEditorPage)
     .WithDescription("Menus to add, change and delete the sections of the "
         "text files: trainers, classes, wild Pokemon, shops, items...")
     .Add("Read The Files Again", ReadFilesAgain)
     .WithDescription("Reads the text files again. The map loads again at "
         "your next step, with the new characters and wild Pokemon.")
     .Add("Live Reload", files.is_live)
     .WithDescription("Reads the files again when you save them, for "
         "example from a computer with an FTP server.")
     .Add("Trace story.txt", files.trace_story)
     .WithDescription("Writes each command of story.txt to the log when it "
         "runs: see Plugin > Log.")
     .Add("Log The Shop Ids", pokemon::CustomShop::GetInstance().log_shop_ids)
     .WithDescription("Writes the id of each shop that opens to the log: "
         "the N of [shop N] in shops.txt.")
     .AddSection("Problems Of The Files");
  const u32 count = DataFiles::GetProblemCount();
  const u32 shown = count < DataFiles::kMaxProblems ? count
                                                    : DataFiles::kMaxProblems;
  for (u32 i = 0; i < shown; i++) {
    const c8* problem = DataFiles::GetProblem(i);
    app.Add(problem).WithDescription(problem);
  }
  if (count > shown) {
    app.Add("More problems: see the log.")
       .WithDescription("Correct the first problems, then read the files "
           "again.");
  }
  if (count == 0) app.Add("No problem.");

  app.AddSection("Export To The Data Folder");
#ifdef GAME_ORAS
  app.Add("Export The Wild Pokemon", [](void*) {
       ShowExportResult(DataExport::WriteEncounters(),
                        "export_encounters.txt");
     })
     .WithDescription("Writes the wild Pokemon of this map to "
         "export_encounters.txt, in the format of encounters.txt.");
#endif
  app.AddSpecies("Species To Export", export_species)
     .Add("Export The Species", [](void*) {
       ShowExportResult(
           DataExport::WriteSpecies(static_cast<SpeciesId>(export_species)),
           "export_species.txt");
     })
     .WithDescription("Writes the stats, the types, the abilities and the "
         "catch rate to export_species.txt.")
     .Add("Export Your Team", [](void*) {
       ShowExportResult(DataExport::WriteTeam(), "export_team.txt");
     })
     .WithDescription("Writes your team to export_team.txt, in the format of "
         "the Pokemon of trainers.txt.");

  app.AddSection("Flags And Variables")
     .Add("Flag", flag_number)
     .WithBounds(0, HackSave::kFlagCount - 1)
     .WithCallback(ReadValues)
     .WithDescription("The number of a flag of story.txt (set flag, if "
         "flag).")
     .Add("Flag Is Set", flag_value)
     .WithCallback([](void*) { HackSave::SetFlag(flag_number, flag_value); })
     .Add("Variable", variable_number)
     .WithBounds(0, HackSave::kVariableCount - 1)
     .WithCallback(ReadValues)
     .WithDescription("The number of a variable of story.txt (set var, if "
         "var).")
     .Add("Variable Value", variable_value)
     .WithCallback([](void*) {
       HackSave::SetVariable(variable_number, variable_value);
     });
}
} // namespace ui
