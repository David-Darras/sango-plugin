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
 * @file auto_surf.cc
 * @brief The player surfs without the Surf menu.
 *
 * The declarations are in overworld/patch/auto_surf.h.
 */

#include "overworld/patch/auto_surf.h"
#include "core/hook_manager.h"
#include "core/native/game_manager.h"
#include "overworld/address.h"

namespace overworld {

namespace {
constexpr u32 kGameManagerEventManager = 0x18;
constexpr u32 kGameManagerGameData = 0x1C;
constexpr u32 kGameDataParty = 0x14;
constexpr u32 kPlayerModel = 0xC;
constexpr u32 kRequestKeyDirection = 0x34;
} // namespace

void AutoSurf::Initialize() {
  core::HookManager::Initialize(HookId::kPlayerCheckPushEvent,
                                address::kPlayerCheckPushEvent,
                                (uptr)CheckPushEventHook, false);
}

void AutoSurf::PatchLoad() {
  core::HookManager::ForceEnable(HookId::kPlayerCheckPushEvent);
}

u32 AutoSurf::CheckPushEventHook(uptr player, uptr event_manager,
                                 const u8* request) {
  const u32 handled = core::HookManager::Call<u32>(
      HookId::kPlayerCheckPushEvent, player, event_manager, request);
  if (handled || !GetInstance().is_enabled) return handled;

  const u32 facing = ((u32(*)(uptr))address::kMoveModelGetFacing)(
      READ32(player + kPlayerModel));
  if (*(const u16*)(request + kRequestKeyDirection) != facing) return 0;
  if (((u32(*)(uptr, u32))address::kPlayerIsSwimOn)(player, facing) == 0) {
    return 0;
  }

  const uptr game_manager = (uptr)&core::GameManager::GetInstance();
  const uptr party =
      READ32(READ32(game_manager + kGameManagerGameData) + kGameDataParty);
  const uptr mount = ((uptr(*)(uptr, u32))address::kPartyGetMember)(party, 0);

  ((void (*)(uptr, uptr, uptr, bool))address::kPlayerCallSwimOnEvent)(
      player, READ32(game_manager + kGameManagerEventManager), mount, false);
  return 1;
}

} // namespace overworld
