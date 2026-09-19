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

#include "ui/patch/title_screen.h"

#include "core/hook_manager.h"

namespace ui {

void TitleScreen::Initialize() {
  core::HookManager::Initialize(HookId::kTitleSequenceSync,
                          core::address::kTitleScreenSequenceSync,
                          (uptr)SequenceSyncHook, false);
}

void TitleScreen::PatchLoad() {
  auto& title = GetInstance();
  if (!title.is_enabled || core::address::kTitleScreenTopVideoId == 0) return;

  MEMORY_SCOPE(sys::address::kMemoryRegionTitleScreen, 0x1E000);
  title.skipped_ = false;
  core::HookManager::ForceEnable(HookId::kTitleSequenceSync);
  WRITE32(core::address::kTitleScreenTopVideoId,
          0xE3A02000 | static_cast<u8>(title.top_video));
  WRITE32(core::address::kTitleScreenBottomVideoId,
          0xE3A02000 | static_cast<u8>(title.bottom_video));
  WRITE32(core::address::kTitleScreenPokemonCrySpecies,
          static_cast<u16>(title.pokemon_cry_species));
  WRITEF(core::address::kTitleScreenPokemonCryVolume, title.pokemon_cry_volume);
  if (title.no_delay)
    ARM_NOP(core::address::kTitleScreenDelay);
  if (title.no_shadow)
    ARM_NOP(core::address::kTitleScreenShadow);
}

void TitleScreen::SequenceSyncHook(uptr display) {
  auto& title = GetInstance();
  if (title.skip_to_frame != 0 && !title.skipped_ &&
      READ32(display + kTopFrameOffset) > 0) {
    title.skipped_ = true;
    WRITE32(display + kTopFrameOffset,
            READ32(display + kTopFrameOffset) + title.skip_to_frame);
    WRITE32(display + kBottomFrameOffset,
            READ32(display + kBottomFrameOffset) + title.skip_to_frame);
  }
  core::HookManager::Call<void>(HookId::kTitleSequenceSync, display);
}

} // namespace ui
