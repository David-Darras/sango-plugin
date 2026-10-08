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
 * @file overworld.h
 * @brief The overworld: a callback when it loads, and the background music.
 */

#pragma once

#include <type_traits>

#include "common.h"
#include "system/constant/background_music.h"

namespace overworld {

/// The settings of overworld::Overworld.
struct OverworldSettings {
  bool freeze_background_music = false; ///< true: always play `background_music`.
  BackgroundMusicId background_music = BackgroundMusicId::kPokemonTheme; ///< The music when freeze_background_music is true.
};
static_assert(std::is_standard_layout<OverworldSettings>::value,
              "OverworldSettings must have standard layout");

/// Calls the callback of the product when the overworld loads. Can replace the music.
struct Overworld : public OverworldSettings {
  MAKE_SINGLETON(Overworld)

  /// Called each time the overworld loads, before the patches of the
  /// library. The code of the overworld CRO is writable.
  typedef void (*OverworldLoadCallback)();
  OverworldLoadCallback on_overworld_load = nullptr;

  static void Initialize();
  /// Called when the overworld loads.
  static void PatchLoad();
  /// The hook that selects the music of a map.
  static u32 GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form);
};
} // namespace overworld
