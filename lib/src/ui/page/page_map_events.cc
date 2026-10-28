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
 * @file page_map_events.cc
 * @brief The menu pages of the signs and the triggers of the map.
 */

#include <cstdio>

#include "overworld/patch/trigger_editor.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using overworld::SignEvent;
using overworld::TriggerEditor;
using overworld::TriggerEvent;

// The menu has 64 entries at most: a page lists the first events.
constexpr u32 kListSize = 20;
constexpr u32 kLabelLength = 56;

c8 sign_labels[kListSize][kLabelLength];
c8 trigger_labels[kListSize][kLabelLength];
c8 map_note[kLabelLength];
c8 record_note[kLabelLength];
c8 more_note[kLabelLength];

// The event of the page of a sign or of a trigger.
u32 selected_index = 0;

const c8* kTalkFrom[] = {"Down", "Left", "Right", "Up",
                         "All Sides", "Left Or Right", "Up Or Down"};

void* ToArgument(u32 index) {
  return reinterpret_cast<void*>(static_cast<uptr>(index + 1));
}

void AddSign(void*) {
  SignEvent* sign = TriggerEditor::AddSign();
  if (sign == nullptr) {
    MainApplication::GetInstance().ShowToast("The table of signs is full.");
    return;
  }
  MainApplication::GetInstance().Refresh();
}

void AddTrigger(void*) {
  TriggerEvent* trigger = TriggerEditor::AddTrigger();
  if (trigger == nullptr) {
    MainApplication::GetInstance().ShowToast("The table of triggers is full.");
    return;
  }
  MainApplication::GetInstance().Refresh();
}

void SaveEvents(void*) {
  MainApplication::GetInstance().ShowToast(
      TriggerEditor::Save() ? "Saved to the SD card."
                            : "Not saved: too many changes, or the SD card "
                              "refused.");
}

void ResetEvents(void*) {
  TriggerEditor::ResetMap();
  MainApplication::GetInstance().Refresh();
}

void MoveSign(void*) {
  SignEvent* sign = TriggerEditor::GetSign(selected_index);
  if (sign != nullptr) TriggerEditor::MoveToPlayer(sign);
  MainApplication::GetInstance().Refresh();
}

void MoveTrigger(void*) {
  TriggerEvent* trigger = TriggerEditor::GetTrigger(selected_index);
  if (trigger != nullptr) TriggerEditor::MoveToPlayer(trigger);
  MainApplication::GetInstance().Refresh();
}

void RemoveSign(void*) {
  if (TriggerEditor::RemoveSign(selected_index)) {
    MainApplication::GetInstance().Close();
  }
}

void RemoveTrigger(void*) {
  if (TriggerEditor::RemoveTrigger(selected_index)) {
    MainApplication::GetInstance().Close();
  }
}
} // namespace

void LoadMapEventsPage(MainApplication& app, void* args) {
  if (!TriggerEditor::IsActive()) {
    app.Add("No events for this map.")
       .WithDescription("The editor takes the events when the game loads a "
                        "map, and a map with too many events is left alone. "
                        "Change the map and look again.");
    return;
  }

  snprintf(map_note, kLabelLength, "Map %lu", (unsigned long)TriggerEditor::GetMap());
  snprintf(record_note, kLabelLength, "Changes in the file: %lu",
           (unsigned long)TriggerEditor::GetRecordCount());
  app.AddSection("Map");
  app.Add(map_note)
     .WithDescription("A sign starts a script when you press {A} in front of "
                      "it. A trigger starts a script when you walk in it. "
                      "The changes are live. Save them to keep them.");
  app.Add(record_note);

  const u32 sign_count = TriggerEditor::GetSignCount();
  app.AddSection("Signs");
  for (u32 i = 0; i < sign_count && i < kListSize; i++) {
    const SignEvent* sign = TriggerEditor::GetSign(i);
    const bool is_added = i >= TriggerEditor::GetShippedSignCount();
    snprintf(sign_labels[i], kLabelLength, "%s%lu: %d,%d  script %lu",
             is_added ? "+ " : (TriggerEditor::IsSignChanged(i) ? "* " : ""),
             (unsigned long)(i + 1), (int)sign->tiles.tile_x,
             (int)sign->tiles.tile_z, (unsigned long)sign->id);
    app.Add(sign_labels[i], LoadSignEventPage, ToArgument(i))
       .WithDescription("A plus means an added sign, a star a changed sign. "
                        "The numbers are the tile and the script.");
  }
  if (sign_count > kListSize) {
    snprintf(more_note, kLabelLength, "%lu more signs are not in this list.",
             (unsigned long)(sign_count - kListSize));
    app.Add(more_note);
  }
  app.Add("Add A Sign Here", AddSign)
     .WithDescription("Adds a sign on your tile. It does nothing until you "
                      "give it a script.");

  const u32 trigger_count = TriggerEditor::GetTriggerCount();
  app.AddSection("Triggers");
  for (u32 i = 0; i < trigger_count && i < kListSize; i++) {
    const TriggerEvent* trigger = TriggerEditor::GetTrigger(i);
    const bool is_added = i >= TriggerEditor::GetShippedTriggerCount();
    snprintf(trigger_labels[i], kLabelLength, "%s%lu: %lu,%lu  script %lu",
             is_added ? "+ " : (TriggerEditor::IsTriggerChanged(i) ? "* " : ""),
             (unsigned long)(i + 1), (unsigned long)trigger->tiles.tile_x,
             (unsigned long)trigger->tiles.tile_z, (unsigned long)trigger->id);
    app.Add(trigger_labels[i], LoadTriggerEventPage, ToArgument(i))
       .WithDescription("A plus means an added trigger, a star a changed "
                        "trigger. The numbers are the tile and the script.");
  }
  if (trigger_count > kListSize) {
    snprintf(more_note, kLabelLength, "%lu more triggers are not in this list.",
             (unsigned long)(trigger_count - kListSize));
    app.Add(more_note);
  }
  app.Add("Add A Trigger Here", AddTrigger)
     .WithDescription("Adds a trigger on your tile. It does nothing until you "
                      "give it a script.");

  app.AddSection("File");
  app.Add("Save To The SD Card", SaveEvents)
     .WithDescription("Writes the changes of this map to "
                      "sdmc:/sango/map_events.txt. The game reads them when "
                      "the map loads.");
  app.Add("Reset This Map", ResetEvents)
     .WithConfirm()
     .WithDescription("The map gets its events from the game again. Save to "
                      "delete the saved changes of this map.");
}

void LoadSignEventPage(MainApplication& app, void* args) {
  if (args != nullptr) selected_index = static_cast<u32>(reinterpret_cast<uptr>(args)) - 1;
  SignEvent* sign = TriggerEditor::IsActive() ? TriggerEditor::GetSign(selected_index)
                                              : nullptr;
  if (sign == nullptr) {
    app.Add("This sign does not exist any more.")
       .WithDescription("Go back and open the list again.");
    return;
  }

  app.AddSection("Sign");
  app.Add("Script", sign->id)
     .WithDescription("The number of the script. 65535: no script. A number "
                      "that the game does not know can freeze the game.")
     .Add("Tile X", sign->tiles.tile_x)
     .Add("Tile Z", sign->tiles.tile_z)
     .Add("Width", sign->tiles.width)
     .WithDescription("The number of tiles of the zone, on the axis X.")
     .Add("Depth", sign->tiles.depth)
     .WithDescription("The number of tiles of the zone, on the axis Z.")
     .Add("Height", sign->tiles.height)
     .WithDescription("The height of the ground. The zone works for a player "
                      "within one tile above or below.")
     .Add("Talk From", reinterpret_cast<u8&>(sign->talk_facing))
     .WithArray(kTalkFrom, SIZE(kTalkFrom))
     .WithDescription("The side where the player must stand.")
     .Add("Kind", sign->kind)
     .WithBounds(0, 10)
     .WithDescription("0: a normal sign. 1: only from the front. 2 to 10: "
                      "the player must look at it (boards, trees, circles).");
  app.Add("Move To Player", MoveSign)
     .WithDescription("Puts the zone on your tile.");
  if (selected_index >= TriggerEditor::GetShippedSignCount()) {
    app.Add("Remove This Sign", RemoveSign)
       .WithConfirm()
       .WithDescription("Removes this added sign. A sign of the game stays.");
  }
}

void LoadTriggerEventPage(MainApplication& app, void* args) {
  if (args != nullptr) selected_index = static_cast<u32>(reinterpret_cast<uptr>(args)) - 1;
  TriggerEvent* trigger = TriggerEditor::IsActive()
                              ? TriggerEditor::GetTrigger(selected_index)
                              : nullptr;
  if (trigger == nullptr) {
    app.Add("This trigger does not exist any more.")
       .WithDescription("Go back and open the list again.");
    return;
  }

  app.AddSection("Trigger");
  app.Add("Script", trigger->id)
     .WithDescription("The number of the script. 65535: no script. A number "
                      "that the game does not know can freeze the game.")
     .Add("Tile X", trigger->tiles.tile_x)
     .Add("Tile Z", trigger->tiles.tile_z)
     .Add("Width", trigger->tiles.width)
     .WithDescription("The number of tiles of the zone, on the axis X.")
     .Add("Depth", trigger->tiles.depth)
     .WithDescription("The number of tiles of the zone, on the axis Z.")
     .Add("Height", trigger->tiles.height)
     .WithDescription("The height of the ground. The zone works for a player "
                      "within one tile above or below.")
     .Add("When", trigger->check_kind)
     .WithBounds(0, 12)
     .WithDescription("0: when the player walks in the zone. 5 to 8: when the "
                      "player pushes against it, up, down, left, right. 9 to "
                      "12: the four diagonals.");
  app.AddSection("Condition")
     .Add("Work", trigger->work_id)
     .WithDescription("0: the trigger always works. Another number: the "
                      "trigger works only when this work of the game has "
                      "the value below (the story flags are works).")
     .Add("Value", trigger->param);
  app.Add("Move To Player", MoveTrigger)
     .WithDescription("Puts the zone on your tile.");
  if (selected_index >= TriggerEditor::GetShippedTriggerCount()) {
    app.Add("Remove This Trigger", RemoveTrigger)
       .WithConfirm()
       .WithDescription("Removes this added trigger. A trigger of the game "
                        "stays.");
  }
}

} // namespace ui
