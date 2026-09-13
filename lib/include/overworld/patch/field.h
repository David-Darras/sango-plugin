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

#include <type_traits>

#include "common.h"
#include "system/constant/background_music.h"

namespace overworld {
class MapManager;

struct FieldSettings {
  bool freeze_background_music = false;
  BackgroundMusicId background_music = BackgroundMusicId::kPokemonTheme;
};
static_assert(std::is_standard_layout<FieldSettings>::value,
              "FieldSettings must have standard layout");

struct Field : public FieldSettings {
  MAKE_SINGLETON(Field)

  /// Called every time the field is (re)loaded, before the built-in
  /// patches, with the game code writable.
  typedef void (*FieldLoadCallback)();
  FieldLoadCallback on_field_load = nullptr;

  static void Initialize();
  static void UpdateZone(MapManager* manager);
  static void PatchLoad();
  static u32 GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form);
};
} // namespace overworld
