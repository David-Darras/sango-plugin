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
 * @file game_extension.h
 * @brief Adds new moves and new abilities to the game.
 *
 * The game has a fixed list of moves and abilities. This feature adds
 * entries after the end of these lists. A new entry has:
 * - a name and a description (the game shows them in the menus),
 * - its data (power, accuracy, type...), for a move,
 * - its battle effect, as a list of reactions (see battle::ReactionTable).
 *
 * Your product registers its moves and abilities in its Initialize()
 * function, with AddMove() and AddAbility().
 *
 * @see docs/tutorials/04-add-a-move.md
 * @see docs/tutorials/05-add-an-ability.md
 */

#pragma once
#include "common.h"
#include "battle/native/reaction_table.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/move.h"
#include "pokemon/native/database.h"

namespace pokemon {
struct MoveData;
}

namespace battle {
struct Pokemon;

/**
 * @brief The description of a new ability.
 *
 * Give an id that the game does not use: an id between
 * AbilityId::kCount and 255.
 */
struct AbilitySpec {
  /// The id of the new ability.
  AbilityId id;
  /// The name that the game shows. Use u"..." text.
  const c16* name;
  /// The description that the game shows. Use "\n" for a new line.
  const c16* description;
  /// The reactions of the ability: what it does, and when.
  const ReactionTable* reactions;
  /// The number of entries in @ref reactions.
  u32 reaction_count;
};

/**
 * @brief The description of a new move.
 *
 * Give an id that the game does not use: an id above MoveId::kCount.
 * The game loads the data of Pound (move 1) for a new move. Then
 * @ref patch_data changes this data.
 */
struct MoveSpec {
  /// The id of the new move.
  MoveId id;
  /// The name that the game shows. Use u"..." text.
  const c16* name;
  /// The description that the game shows. Use "\n" for a new line.
  const c16* description;
  /// Sets the data of the move (power, accuracy, PP, type...). Can be null.
  void (*patch_data)(pokemon::MoveData& move);
  /// Selects the battle animation of the move. Can be null.
  /// Set `id` to the animation to play. Set `is_move` to false when `id` is
  /// not a move id. You can also use battle::MoveAnimations::Define() to make
  /// a new animation.
  void (*patch_animation)(u32& id, bool& is_move);
  /// The reactions of the move: what it does in battle, and when.
  const ReactionTable* reactions;
  /// The number of entries in @ref reactions.
  u32 reaction_count;
};

/**
 * @brief Adds new moves and new abilities to the game.
 *
 * The library calls Initialize() at start-up. Your product calls AddMove()
 * and AddAbility().
 *
 * @code
 * static const battle::ReactionTable kMyMoveReactions[] = {
 *     {battle::MomentKind::kMoveExecutionStart, MyMoveReaction},
 * };
 *
 * battle::GameExtension::AddMove({kMyMove, u"My Move", u"A new move.",
 *                                 PatchMyMoveData, nullptr,
 *                                 kMyMoveReactions, SIZE(kMyMoveReactions)});
 * @endcode
 */
class GameExtension {
  MAKE_SINGLETON(GameExtension)
public:
  /// The maximum number of new moves.
  static constexpr u32 kMaxMoves = 32;
  /// The maximum number of new abilities.
  static constexpr u32 kMaxAbilities = 32;

  /// Installs the hooks. The library calls this function at start-up.
  static void Initialize();

  /**
   * @brief Adds a new move.
   * @param spec The description of the move. The function keeps a copy.
   * @return false when the list is full or when the id is already in use.
   */
  static bool AddMove(const MoveSpec& spec);

  /**
   * @brief Adds a new ability.
   * @param spec The description of the ability. The function keeps a copy.
   * @return false when the list is full or when the id is already in use.
   */
  static bool AddAbility(const AbilitySpec& spec);

  /// Returns the new move with this id, or null.
  static const MoveSpec* FindMove(MoveId id);

  /// Returns the new ability with this id, or null.
  static const AbilitySpec* FindAbility(AbilityId id);

private:
  static constexpr u32 kMoveAnimationCount = GAME_CONSTANT(0x26A, 0);

  static void BattleLoadAnimationHook(uptr self, u32 id, bool is_move);
  static void BattleLoadEffectHook(uptr self, u32 archive_id, u32 file_id,
                                   u32 type);
  static u32 LoadMoveData(uptr self, MoveId move_id);

  static bool PatchMoveName(MoveId move, String* output);
  static bool PatchMoveDescription(MoveId move, String* output);
  static bool PatchAbilityName(AbilityId ability, String* output);
  static bool PatchAbilityDescription(AbilityId ability, String* output);

  static uptr GetBattleAbilityHandlerHook(Pokemon* pkm);
  static uptr GetBattleMoveHandlerHook(Pokemon* pkm, MoveId move, u32 x);

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

  MoveSpec moves_[kMaxMoves];
  u32 move_count_ = 0;
  AbilitySpec abilities_[kMaxAbilities];
  u32 ability_count_ = 0;
};

} // namespace battle
