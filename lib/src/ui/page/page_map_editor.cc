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
 * @file page_map_editor.cc
 * @brief The menu page of the map editor: opens the touch editor and manages
 *        the changes of the world.
 */

#include <cstdio>

#include "core/native/process_manager.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/map_editor.h"
#include "overworld/patch/trigger_editor.h"
#include "script/patch/data_scripts.h"
#include "ui/main_application.h"
#include "ui/map_edit_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using overworld::MapEditor;

constexpr u32 kLabelLength = 64;
c8 world_note[kLabelLength];
c8 run_note[kLabelLength];
c8 tile_note[kLabelLength];

void SaveEdits(void*) {
  MainApplication::GetInstance().ShowToast(
      MapEditor::Save() ? "Saved to the SD card." : "The SD card refused.");
  MainApplication::GetInstance().Refresh();
}

void ClearEdits(void*) {
  MapEditor::ClearAll();
  // The game builds the tiles again from its own data.
  const overworld::Position& position =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  overworld::MapManager::ChangeMap(overworld::MapManager::GetInstance().GetMap(),
                                   position, overworld::Facing::kUp, true,
                                   false);
  MainApplication::GetInstance().ShowToast("The tiles are the ones of the game.");
  MainApplication::GetInstance().Refresh();
}

void ExportText(void*) {
  MainApplication::GetInstance().ShowToast(
      MapEditor::ExportText() ? "Written: sdmc:/sango/maps/layout_N.txt"
                              : "Go to the overworld first.");
}

void ImportText(void*) {
  u32 changed = 0;
  if (!MapEditor::ImportText(&changed)) {
    MainApplication::GetInstance().ShowToast("No text file for this world.");
    return;
  }
  c8 message[48];
  snprintf(message, sizeof(message), "%lu tiles changed.", (unsigned long)changed);
  MainApplication::GetInstance().ShowToast(message);
  MainApplication::GetInstance().Refresh();
}

// One switch for all the files: the tiles, the events and the scripts.
void ApplyLiveReload(void*) {
  const bool is_live = MapEditor::GetInstance().is_live;
  overworld::TriggerEditor::GetInstance().is_live = is_live;
  script::DataScripts::GetInstance().is_live = is_live;
}

void OpenEditor(void*) {
  if (!core::ProcessManager::IsOverworldActive()) {
    MainApplication::GetInstance().ShowToast("Go to the overworld first.");
    return;
  }
  MapEditApplication::Open();
}
} // namespace

void LoadMapEditorPage(MainApplication& app, void* args) {
  auto& editor = MapEditor::GetInstance();
  const auto& layout = overworld::WorldLayout::GetInstance();
  snprintf(world_note, kLabelLength, "World %lu: %lu x %lu blocks of 40 tiles",
           (unsigned long)layout.id, (unsigned long)layout.width,
           (unsigned long)layout.height);
  snprintf(run_note, kLabelLength, "Runs: %lu of %lu",
           (unsigned long)MapEditor::GetRunCount(),
           (unsigned long)MapEditor::kMaxRuns);
  snprintf(tile_note, kLabelLength, "Changed tiles: %lu%s",
           (unsigned long)MapEditor::GetTileCount(),
           MapEditor::IsDirty() ? "  (not saved)" : "");

  app.AddSection("Tiles");
  app.Add("Open The Map Editor", OpenEditor)
     .WithDescription("Shows the map on the bottom screen. Touch it to change "
                      "the attributes of the tiles: walls, water, wild "
                      "Pokemon, the type of the ground. Use the World "
                      "Overlay to see the result on the top screen.");
  app.Add(world_note);
  app.Add(run_note)
     .WithDescription("A run is a row of tiles with the same attribute. The "
                      "editor keeps 1536 runs for a world.");
  app.Add(tile_note);

  app.AddSection("Events, Scripts And Zone");
  app.Add("Map Events", LoadMapEventsPage)
     .WithDescription("The list of the signs and the triggers of the map. "
                      "The touch editor has the mode Events for all the "
                      "events.");
  app.Add("Scripts Without Code", LoadDataScriptsPage)
     .WithDescription("Write scripts with steps: show a text, give an item, "
                      "set a flag... A sign or a trigger starts them.");
  app.Add("Music, Weather And Time Of This Map", LoadZoneSettingsPage)
     .WithDescription("Gives the current map its own music, weather and time "
                      "of day.");

  app.AddSection("Text Files And Live Reload");
  app.Add("Export The Tiles To Text", ExportText)
     .WithDescription("Writes the 128 x 128 tiles around you to a text file: "
                      "# wall, ~ water, comma wild Pokemon, . plain. Open it "
                      "on a computer.");
  app.Add("Import The Tiles From Text", ImportText)
     .WithDescription("Reads the text file and changes the tiles that are "
                      "different.");
  app.OnChange(ApplyLiveReload);
  app.Add("Live Reload Of The Files", editor.is_live)
     .WithDescription("On: the game reads again the text file of the tiles, "
                      "the events file and the scripts file each time that "
                      "you save them on a computer. Put the SD card files "
                      "where the plugin reads them.");

  app.AddSection("Changes");
  app.Add("Apply The Changes", editor.is_enabled)
     .WithDescription("Off: the game uses its own tiles. The changes stay in "
                      "the editor.");
  app.Add("Save When Leaving", editor.auto_save)
     .WithDescription("On: the editor writes the file when you close it and "
                      "when you go to another world.");
  app.Add("Save To The SD Card", SaveEdits)
     .WithDescription("Writes sdmc:/sango/maps/layout_<world>.tiles. The game "
                      "reads it when you enter the world.");
  app.Add("Clear All The Changes Of This World", ClearEdits)
     .WithConfirm()
     .WithDescription("Deletes the changes and the file of this world, and "
                      "loads the map again so that the tiles are the ones of "
                      "the game.");
}

} // namespace ui
