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
 * @file trigger_editor.h
 * @brief Adds and changes the signs and the triggers of the map.
 */

#pragma once

#include "common.h"
#include "overworld/native/map_event_data.h"

namespace overworld {

/**
 * @brief Adds and changes the signs and the triggers of the map.
 *
 * A **sign** starts a script when the player presses {A} in front of a zone
 * of tiles. A **trigger** starts a script when the player walks in a zone of
 * tiles, or pushes against it. The game reads the events of the map through
 * the tables of MapEventData. When a map loads, the editor copies
 * the tables of the game into its own tables (a table of the game cannot
 * grow), then applies the changes of the SD card. A change of a table takes
 * effect at once.
 *
 * A change stays in the file `sdmc:/sango/map_events.txt` when the player
 * saves it: the changes of the shipped events (by their number in the table)
 * and the added events. The editor does not remove the events of the game.
 *
 * Warning: a script number that the game does not know can freeze the game
 * when the event starts. 65535 means no script: the event does nothing.
 */
class TriggerEditor {
  MAKE_SINGLETON(TriggerEditor)
public:
  static constexpr u32 kMaxSigns = 40;
  static constexpr u32 kMaxTriggers = 40;
  /// The changes that the file keeps, for all the maps.
  static constexpr u32 kMaxRecords = 64;
  /// The script number of an event that does nothing.
  static constexpr u16 kNoScript = 0xFFFF;

  /// Reads the changes of the SD card.
  static void Initialize();

  /// Called when the game loads the events of a map: takes the tables.
  static void OnMapEventsLoaded(MapEventData* events);

  /// Returns true when the editor holds the events of the current map.
  static bool IsActive();
  /// The events of the current map (with the tables of the editor), or null.
  static const MapEventData* GetEvents();
  static u32 GetMap();

  static u32 GetSignCount();
  /// The number of signs of the game. The next ones are added signs.
  static u32 GetShippedSignCount();
  static SignEvent* GetSign(u32 index);
  static u32 GetTriggerCount();
  static u32 GetShippedTriggerCount();
  static TriggerEvent* GetTrigger(u32 index);

  /// Adds a sign on the tile of the player. The sign does nothing until it
  /// has a script. Returns null when the table is full.
  static SignEvent* AddSign();
  static TriggerEvent* AddTrigger();

  /// Puts the zone of an event on the tile of the player.
  static void MoveToPlayer(SignEvent* sign);
  static void MoveToPlayer(TriggerEvent* trigger);

  /// Removes an added event. Returns false for an event of the game.
  static bool RemoveSign(u32 index);
  static bool RemoveTrigger(u32 index);

  /// Returns true when the event is different from the one of the game.
  static bool IsSignChanged(u32 index);
  static bool IsTriggerChanged(u32 index);

  /// Writes the changes of the current map to the SD card (the changes of the
  /// other maps stay). Returns false when the file is full or cannot be
  /// written.
  static bool Save();

  /// Gives the current map the events of the game again, and forgets the
  /// saved changes of this map (the file changes at the next Save()).
  static void ResetMap();

  static u32 GetRecordCount();

private:
  enum RecordKind : u8 { kAddedSign, kAddedTrigger, kEditedSign, kEditedTrigger };

  struct Record {
    bool is_used = false;
    RecordKind kind = kAddedSign;
    u16 map = 0;
    u16 index = 0;
    SignEvent sign = {};
    TriggerEvent trigger = {};
  };

  static bool LoadLine(const c8* line);
  void ApplyRecords();
  void TakeTables();
  Record* NewRecord();

  MapEventData* events_ = nullptr;
  u16 map_ = 0;
  u16 sign_count_ = 0;
  u16 trigger_count_ = 0;
  u16 shipped_sign_count_ = 0;
  u16 shipped_trigger_count_ = 0;
  SignEvent signs_[kMaxSigns] = {};
  TriggerEvent triggers_[kMaxTriggers] = {};
  SignEvent shipped_signs_[kMaxSigns] = {};
  TriggerEvent shipped_triggers_[kMaxTriggers] = {};
  Record records_[kMaxRecords];
};

} // namespace overworld
