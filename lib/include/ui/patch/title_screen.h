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
#include "pokemon/constant/species.h"
#include "ui/constant/video.h"
#include <type_traits>

namespace ui {

struct TitleScreenSettings {
  bool is_enabled = true;
  bool no_delay = false;
  bool no_shadow = false;
  VideoId top_video = VideoId::kTitleOmegaRuby;
  VideoId bottom_video = VideoId::kPrimoGroudon;
  SpeciesId pokemon_cry_species = SpeciesId::kGroudon;
  f32 pokemon_cry_volume = 0.8f;
  u32 skip_to_frame = 0;
};
static_assert(std::is_standard_layout<TitleScreenSettings>::value,
              "TitleScreenSettings must have standard layout");

class TitleScreen : public TitleScreenSettings {
  MAKE_SINGLETON(TitleScreen)

public:
  static void Initialize();
  static void PatchLoad();

private:
  static constexpr u32 kTopFrameOffset = 0x98;
  static constexpr u32 kBottomFrameOffset = 0x9C;

  static void SequenceSyncHook(uptr display);

  bool skipped_ = false;
};

} // namespace ui
