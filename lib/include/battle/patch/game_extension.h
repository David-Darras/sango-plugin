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
#include "battle/native/listener.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/move.h"
#include "pokemon/native/database.h"

// Plugin-defined extensions to the game's ability / move ID spaces. They are
// not part of the vanilla tables in pokemon/constant, so they are declared
// here as typed constants rather than added to the enums themselves.
constexpr AbilityId kAbilityToxicDrizzle = static_cast<AbilityId>(255);
constexpr AbilityId kAbilityRadioactiveDrizzle = static_cast<AbilityId>(254);
constexpr AbilityId kAbilityRealityWarp = static_cast<AbilityId>(253);
constexpr MoveId kMoveAbsoluteZero = static_cast<MoveId>(863);
constexpr MoveId kMoveSolarFlare = static_cast<MoveId>(864);

namespace pokemon {
struct MoveData;
}

namespace battle {
struct Pokemon;

struct AbilitySpec {
  AbilityId id;
  const c16* name;
  const c16* description;
  const ReactionTable* reactions;
  u32 reaction_count;
};

struct MoveSpec {
  MoveId id;
  const c16* name;
  const c16* description;
  void (*patch_data)(pokemon::MoveData& move);
  void (*patch_animation)(u32& id, bool& is_move);
  const ReactionTable* reactions;
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
  static bool PatchAbilityName(AbilityId ability, String* output);
  static bool PatchAbilityDescription(AbilityId ability, String* output);

  static uptr GetBattleAbilityHandlerHook(Pokemon* pkm);
  static uptr GetBattleMoveHandlerHook(Pokemon* pkm, MoveId move,
                                       u32 x);

  static void MessageGetStringHook(Message* self, u32 str_id, String* output);
  static void SetAbilityNameHook(uptr self, u32 archive, u32 ability);
  static void SetMoveNameHook(uptr self, u32 archive, u32 move);
  static void GetAbilityNameHook(String* output, AbilityId ability);
  static void GetMoveNameHook(MoveId move, String* output);
  static void GetAbilityDescriptionHook(String* output, AbilityId ability);

  STATIC_INLINE Message& GetAbilityName() {
    return *pokemon::Database::GetInstance().ability_names;
  }

  STATIC_INLINE Message& GetAbilityDescription() {
    return *pokemon::Database::GetInstance().ability_descriptions;
  }

  STATIC_INLINE Message& GetMoveName() {
    return *pokemon::Database::GetInstance().move_names;
  }
};

} // namespace battle
