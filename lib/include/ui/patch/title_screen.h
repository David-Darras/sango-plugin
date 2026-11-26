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
 * @file title_screen.h
 * @brief Changes the title screen.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"
#include "ui/constant/video.h"
#include <type_traits>

namespace ui {

/// The settings of ui::TitleScreen. With the values that they have here, the
/// title screen is the one of the game (Alpha Sapphire).
struct TitleScreenSettings {
  bool no_delay = false;
  bool no_shadow = false;
  VideoId top_video = VideoId::kTitleAlphaSapphire;
  VideoId bottom_video = VideoId::kPrimoKyogre;
  SpeciesId pokemon_cry_species = SpeciesId::kKyogre;
  f32 pokemon_cry_volume = 0.7f;
  u32 skip_to_frame = 0;
};
static_assert(std::is_standard_layout<TitleScreenSettings>::value,
              "TitleScreenSettings must have standard layout");

/// Changes the title screen: video, cry, delay.
class TitleScreen : public TitleScreenSettings {
  MAKE_SINGLETON(TitleScreen)

public:
  static void Initialize();
  static void PatchLoad();
  /// Returns true when a setting is not the one of the game.
  static bool IsChanged();

private:
  static constexpr u32 kTopFrameOffset = 0x98;
  static constexpr u32 kBottomFrameOffset = 0x9C;

  static void SequenceSyncHook(uptr display);

  bool skipped_ = false;
};

} // namespace ui
