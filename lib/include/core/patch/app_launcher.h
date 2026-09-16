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

#pragma once

#include "common.h"
#include "core/constant/app_id.h"

namespace core {
class GameManager;

/// Opens one of the game's applications from the overworld menu, filling in
/// the input the game expects for it.
class AppLauncher {
  MAKE_SINGLETON(AppLauncher)

public:
  static void Initialize();
  void TriggerApp(AppId id);
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
