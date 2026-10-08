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
 * @file auto_surf.h
 * @brief The player surfs without the Surf menu.
 */

#pragma once

#include "common.h"

namespace overworld {

/// The player starts to surf when the player walks into water.
struct AutoSurf {
  MAKE_SINGLETON(AutoSurf)

  bool is_enabled = true;

  static void Initialize();
  static void PatchLoad();

private:
  static u32 CheckPushEventHook(uptr player, uptr event_manager,
                                const u8* request);
};
} // namespace overworld
