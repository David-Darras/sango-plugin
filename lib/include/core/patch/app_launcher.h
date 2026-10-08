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
 * @file app_launcher.h
 * @brief Opens the applications of the game (PC box, Town Map, Move Reminder...) from the menu.
 */

#pragma once

#include "common.h"
#include "core/constant/app_id.h"

namespace core {
class GameManager;

/// Opens an application of the game from the overworld menu. It fills the
/// input that the application needs.
class AppLauncher {
  MAKE_SINGLETON(AppLauncher)

public:
  static void Initialize();
  /// Opens an application at the next frame of the overworld menu.
  void TriggerApp(AppId id);
  /// Opens the Town Map in Fly mode.
  static void DoFly();

private:
  static bool CheckAppRequestHook(uptr menu, u32 id);
  static void MoveDeleterCallback(uptr* data, GameManager* manager);
  static void MoveTutorCallback(uptr* data, GameManager* manager);
  static void TownMapCallback(uptr* data, GameManager* manager);
  static void CallAppHook(uptr self, GameManager* manager);

  bool open_app = false;
  AppId app_id = AppId::kBox;
};
} // namespace core
