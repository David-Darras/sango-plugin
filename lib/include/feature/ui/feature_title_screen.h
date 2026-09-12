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
#include "feature/core/hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/species.h"
#include "game/constant/video.h"
#include "game/renderer/pokemon_model.h"
#include "ui/log_application.h"

namespace feature {
class TitleScreen {
  MAKE_SINGLETON(TitleScreen)
  bool is_enabled = true;
  bool no_delay = false;
  bool no_shadow = false;
  Video top_video = Video::kTitleOmegaRuby;
  Video bottom_video = Video::kPrimoGroudon;
  Species pokemon_cry_species = Species::kGroudon;
  f32 pokemon_cry_volume = 0.8f;
  u32 skip_to_frame = 0;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kTitleSequenceSync,
                            ADDRESS_TITLE_SCREEN_SEQUENCE_SYNC,
                            (uptr)SequenceSyncHook, false);
  }

  STATIC_INLINE void PatchLoad() {
    auto& title = GetInstance();
    if (!title.is_enabled) return;

    MEMORY_SCOPE(ADDRESS_MEMORY_REGION_TITLE_SCREEN, 0x1E000);
    title.skipped_ = false;
    HookManager::ForceEnable(HookID::kTitleSequenceSync);
    WRITE32(ADDRESS_TITLE_SCREEN_TOP_VIDEO_ID,
            0xE3A02000 | static_cast<u8>(title.top_video));
    WRITE32(ADDRESS_TITLE_SCREEN_BOTTOM_VIDEO_ID,
            0xE3A02000 | static_cast<u8>(title.bottom_video));
    WRITE32(ADDRESS_TITLE_SCREEN_POKEMON_CRY_SPECIES,
            static_cast<u16>(title.pokemon_cry_species));
    WRITEF(ADDRESS_TITLE_SCREEN_POKEMON_CRY_VOLUME, title.pokemon_cry_volume);
    if (title.no_delay)
      ARM_NOP(ADDRESS_TITLE_SCREEN_DELAY);
    if (title.no_shadow)
      ARM_NOP(ADDRESS_TITLE_SCREEN_SHADOW);
  }

private:
  static constexpr u32 kTopFrameOffset = 0x98;
  static constexpr u32 kBottomFrameOffset = 0x9C;

  static void SequenceSyncHook(uptr display) {
    auto& title = GetInstance();
    if (title.skip_to_frame != 0 && !title.skipped_ &&
        READ32(display + kTopFrameOffset) > 0) {
      title.skipped_ = true;
      WRITE32(display + kTopFrameOffset,
              READ32(display + kTopFrameOffset) + title.skip_to_frame);
      WRITE32(display + kBottomFrameOffset,
              READ32(display + kBottomFrameOffset) + title.skip_to_frame);
    }
    HookManager::Call<void>(HookID::kTitleSequenceSync, display);
  }

  bool skipped_ = false;
};
} // namespace feature