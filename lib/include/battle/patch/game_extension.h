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
#include "battle/constant/type_multiplier.h"
#include "battle/native/reaction_table.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/type.h"
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
 * @brief The description of a new type, or of a type of the game that changes.
 *
 * A type has a number. The game has the numbers 0 to 17 (Normal to Fairy), and
 * it uses 18 for "no type". A new type has a number from 19 to 31
 * (TypeId::kFirstCustom to TypeId::kLastCustom). With a number from 0 to 17,
 * the description changes a type of the game: its name and its icon.
 */
struct TypeSpec {
  /// The number of the type.
  TypeId id;
  /// The name that the game shows. Use u"..." text. Null keeps the name (and
  /// the icon) of the type of the game.
  ///
  /// The icon is the file `sdmc:/sango/types/<name>.bclim`, with the same
  /// letters as the name (for example `Cosmic.bclim` for u"Cosmic"). The image
  /// has 50 x 20 pixels and the format RGBA4 (see tools/png_to_bclim.py).
  /// Without the file, a type of the game keeps its icon, and a new type shows
  /// the icon of the type Fairy.
  const c16* name;
  /// For a new type: the type of the game that gives its colour to the buttons
  /// of the moves in a battle (for example TypeId::kDragon).
  TypeId colour_type;
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
 *
 * It also adds new types (AddType() and SetTypeChart()):
 *
 * @code
 * // The icon is sdmc:/sango/types/Cosmic.bclim.
 * battle::GameExtension::AddType({static_cast<TypeId>(19), u"Cosmic",
 *                                 TypeId::kDragon});
 * battle::GameExtension::SetTypeChart(kMyChart, 19);
 * @endcode
 */
class GameExtension {
  MAKE_SINGLETON(GameExtension)
public:
  /// The maximum number of new moves.
  static constexpr u32 kMaxMoves = 32;
  /// The maximum number of new abilities.
  static constexpr u32 kMaxAbilities = 32;
  /// The number of the types: the types of the game, "no type" (18) and the
  /// new types.
  static constexpr u32 kMaxTypes = 32;

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

  /**
   * @brief Adds a new type, or changes the name and the icon of a type of the
   *        game (ORAS only).
   *
   * The type then has its name and its icon in the battle, the summary, the
   * party, the PC box and the menu of the plugin. Its effects in the battle
   * come from the type chart: see SetTypeChart(). Call this function and
   * SetTypeChart() before the game starts a battle.
   *
   * @param spec The description of the type. The function keeps a copy.
   * @return false when the number is 18, when it is above 31 or when the type
   *         is already registered.
   */
  static bool AddType(const TypeSpec& spec);

  /**
   * @brief Gives the type chart of the battles.
   *
   * The table has one row for each attacking type and one column for each
   * defending type: `table[attacking * type_count + defending]`. The first 18
   * numbers are the types of the game, and the next numbers are the new types
   * in the order of their numbers (19 is the number 18 of the table, 20 is the
   * number 19, and so on). The cells of the types of the game go to the memory
   * of the game when a battle loads, and the cells of the new types stay in the
   * plugin.
   *
   * @param table The cells.
   * @param type_count The number of types of the table, from 18 to 31.
   */
  static void SetTypeChart(const TypeMultiplier* table, u32 type_count);

  /// Writes the path of the icon of a type: sdmc:/sango/types/<name>.bclim.
  /// Returns false when the type has no name or when the path is too long.
  static bool GetTypeIconPath(const TypeSpec& spec, c16* path, u32 capacity);

  /// Returns the description of a type, or null.
  static const TypeSpec* FindType(u32 id);

  /// Returns the number of names that a list of types needs: the types of
  /// the game, "no type" and the new types that are registered.
  static u32 GetTypeListSize();

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
  static bool PatchTypeName(u32 type, String* output);

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
  TypeSpec types_[kMaxTypes];
  bool is_type_used_[kMaxTypes] = {};
};

} // namespace battle
