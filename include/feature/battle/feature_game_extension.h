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
#include "game/battle/listener.h"
#include "game/constant/ability.h"
#include "game/constant/move.h"

// Plugin-defined extensions to the game's ability / move ID spaces. They are
// not part of the vanilla tables in game/constant, so they are declared here
// as typed constants rather than added to the enums themselves.
constexpr Ability kAbilityToxicDrizzle = static_cast<Ability>(255);
constexpr Ability kAbilityRadioactiveDrizzle = static_cast<Ability>(254);
constexpr Ability kAbilityRealityWarp = static_cast<Ability>(253);
constexpr MoveId kMoveAbsoluteZero = static_cast<MoveId>(863);
constexpr MoveId kMoveSolarFlare = static_cast<MoveId>(864);

namespace global_data {
struct Move;
}

namespace battle {
struct Pokemon;
}

namespace feature {
struct AbilitySpec {
  Ability id;
  const c16* name;
  const c16* description;
  const battle::ReactionTable* reactions;
  u32 reaction_count;
};

struct MoveSpec {
  MoveId id;
  const c16* name;
  const c16* description;
  void (*patch_data)(global_data::Move& move);
  void (*patch_animation)(u32& id, bool& is_move);
  const battle::ReactionTable* reactions;
  u32 reaction_count;
};

class GameExtension {
  MAKE_SINGLETON(GameExtension)
public:
  static void Initialize();
  static void PatchBattleLoad();

private:
  static u32 BattleAddTerrainHook(u32 a, u32 b);
  static void BattleLoadAnimationHook(uptr self, u32 id, bool is_move);
  static u32 LoadMoveData(uptr self, MoveId move_id);

  static bool PatchMoveName(MoveId move, String* output);
  static bool PatchMoveDescription(MoveId move, String* output);
  static bool PatchAbilityName(Ability ability, String* output);
  static bool PatchAbilityDescription(Ability ability, String* output);

  static uptr GetBattleAbilityHandlerHook(battle::Pokemon* pkm);
  static uptr GetBattleMoveHandlerHook(battle::Pokemon* pkm, MoveId move,
                                       u32 x);

  static void MessageGetStringHook(Message* self, u32 str_id, String* output);
  static void SetAbilityNameHook(uptr self, u32 archive, u32 ability);
  static void SetMoveNameHook(uptr self, u32 archive, u32 move);
  static void GetAbilityNameHook(String* output, Ability ability);
  static void GetMoveNameHook(MoveId move, String* output);
  static void GetAbilityDescriptionHook(String* output, Ability ability);

  STATIC_INLINE Message& GetAbilityName() {
    return *(Message*)READ32(ADDRESS_GLOBAL_DATA_ABILITY_NAME_MESSAGE);
  }

  STATIC_INLINE Message& GetAbilityDescription() {
    return *(Message*)READ32(ADDRESS_GLOBAL_DATA_ABILITY_DESCRIPTION_MESSAGE);
  }

  static Message& GetMoveName() {
    return *(Message*)READ32(ADDRESS_GLOBAL_DATA_MOVE_NAME_MESSAGE);
  }
};
} // namespace feature