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
 * @file item_medicine.h
 * @brief The effects that the game gives to the items of the Bag (medicines,
 *        vitamins, candies...), with names.
 *
 * The Bag applies a medicine to a Pokémon with one function of the game, and the
 * data of the item tells what it does. These functions change the data of an
 * item (`patch_data` of battle::ItemSpec). The game then shows its own messages
 * and the screen of the Pokémon, and removes the item.
 *
 * @code
 * void PatchTonic(pokemon::ItemData& item) {
 *   battle::ItemMedicine::MakeUsable(item);
 *   battle::ItemMedicine::CureAllStatus(item);
 *   battle::ItemMedicine::RestoreHp(item, battle::ItemMedicine::kHalf);
 * }
 * @endcode
 *
 * @see docs/tutorials/15-add-an-item.md
 */

#pragma once

#include "common.h"
#include "pokemon/native/item_data.h"

namespace battle {

/// The statistic of an effort value that an item changes.
enum class EvStat : u8 {
  kHp,
  kAttack,
  kDefense,
  kSpeed,
  kSpecialAttack,
  kSpecialDefense,
};

/// Names for the effects that the game gives to an item in the Bag. All the
/// functions change the data of the item that they get.
class ItemMedicine {
public:
  /// A number of HP: all, half or a quarter of the maximum HP. With these values
  /// the amount depends on the Pokémon. Any other value is a number of HP.
  static constexpr u8 kQuarter = 253;
  static constexpr u8 kHalf = 254;
  static constexpr u8 kAll = 255;

  /// Makes the item usable on a Pokémon in the Bag (the Bag shows "Use"), and
  /// removes all the effects. Call it first, then add the effects.
  static void MakeUsable(pokemon::ItemData& item) {
    item.overworld_function = 1; // the use of a medicine
    item.battle_function = 0;
    item.use_on_pokemon = true;
    item.use_no_consume = 0;
    item.cure_sleep = item.cure_poison = item.cure_burn = 0;
    item.cure_freeze = item.cure_paralysis = 0;
    item.cure_confusion = item.cure_infatuation = 0;
    item.revive = item.revive_all = item.level_up = item.evolve = 0;
    item.use_pp_up = item.use_pp_max = 0;
    item.restore_pp = item.restore_all_pp = item.restore_hp = 0;
    item.raise_hp_ev = item.raise_attack_ev = item.raise_defense_ev = 0;
    item.raise_speed_ev = item.raise_sp_atk_ev = item.raise_sp_def_ev = 0;
    item.ev_limit_control = 0;
    item.friendship_mod_1 = item.friendship_mod_2 = item.friendship_mod_3 = 0;
    item.hp_ev_value = item.attack_ev_value = item.defense_ev_value = 0;
    item.speed_ev_value = item.sp_atk_ev_value = item.sp_def_ev_value = 0;
    item.hp_restore_value = item.pp_restore_value = 0;
    item.friendship_value_1 = item.friendship_value_2 = 0;
    item.friendship_value_3 = 0;
  }

  /// Cures the sleep, the poison, the burn, the freeze and the paralysis (each
  /// argument turns one on or off).
  static void CureStatus(pokemon::ItemData& item, bool sleep, bool poison,
                         bool burn, bool freeze, bool paralysis) {
    item.cure_sleep = sleep;
    item.cure_poison = poison;
    item.cure_burn = burn;
    item.cure_freeze = freeze;
    item.cure_paralysis = paralysis;
  }

  /// Cures the five major status conditions (a Full Heal).
  static void CureAllStatus(pokemon::ItemData& item) {
    CureStatus(item, true, true, true, true, true);
  }

  /// Restores HP: a number, kQuarter, kHalf or kAll. A fainted Pokémon cannot
  /// use the item.
  static void RestoreHp(pokemon::ItemData& item, u8 amount) {
    item.restore_hp = 1;
    item.hp_restore_value = amount;
  }

  /// Revives a fainted Pokémon with a number of HP (a number, kQuarter, kHalf
  /// or kAll). Only a fainted Pokémon can use the item.
  static void Revive(pokemon::ItemData& item, u8 amount) {
    item.revive = 1;
    item.hp_restore_value = amount;
  }

  /// Revives all the fainted Pokémon of the party (a Sacred Ash). Use it with
  /// Revive() for the HP.
  static void ReviveParty(pokemon::ItemData& item) {
    item.revive_all = 1;
  }

  /// Raises the level by one (a Rare Candy).
  static void LevelUp(pokemon::ItemData& item) {
    item.level_up = 1;
  }

  /// Restores the PP of one move that the player chooses (an Ether), or of all
  /// the moves (an Elixir). `amount` is a number of PP; 255 restores all.
  static void RestorePp(pokemon::ItemData& item, u8 amount, bool all_moves) {
    item.restore_pp = all_moves ? 0 : 1;
    item.restore_all_pp = all_moves ? 1 : 0;
    item.pp_restore_value = amount;
  }

  /// Raises the maximum PP of one move that the player chooses: by one step (a
  /// PP Up), or up to three steps (a PP Max).
  static void RaisePp(pokemon::ItemData& item, bool to_maximum) {
    item.use_pp_up = to_maximum ? 0 : 1;
    item.use_pp_max = to_maximum ? 1 : 0;
  }

  /// Changes an effort value (a vitamin: positive; a berry or a wing: see the
  /// game). `amount` goes from -127 to 127. A vitamin stops at 100 effort
  /// values unless IgnoreEvLimit() is on.
  static void ChangeEv(pokemon::ItemData& item, EvStat stat, s8 amount) {
    const u8 on = amount != 0;
    switch (stat) {
      case EvStat::kHp:
        item.raise_hp_ev = on;
        item.hp_ev_value = amount;
        break;
      case EvStat::kAttack:
        item.raise_attack_ev = on;
        item.attack_ev_value = amount;
        break;
      case EvStat::kDefense:
        item.raise_defense_ev = on;
        item.defense_ev_value = amount;
        break;
      case EvStat::kSpeed:
        item.raise_speed_ev = on;
        item.speed_ev_value = amount;
        break;
      case EvStat::kSpecialAttack:
        item.raise_sp_atk_ev = on;
        item.sp_atk_ev_value = amount;
        break;
      case EvStat::kSpecialDefense:
        item.raise_sp_def_ev = on;
        item.sp_def_ev_value = amount;
        break;
    }
  }

  /// With true, the item can raise an effort value above 100 (up to 252).
  static void IgnoreEvLimit(pokemon::ItemData& item, bool is_ignored) {
    item.ev_limit_control = is_ignored;
  }

  /// Changes the friendship. The game uses one number for a Pokémon with a low
  /// friendship (under 100), one for a medium friendship (100 to 199) and one
  /// for a high friendship (200 and more). Each number goes from -127 to 127.
  static void ChangeFriendship(pokemon::ItemData& item, s8 low, s8 medium,
                               s8 high) {
    item.friendship_mod_1 = low != 0;
    item.friendship_mod_2 = medium != 0;
    item.friendship_mod_3 = high != 0;
    item.friendship_value_1 = low;
    item.friendship_value_2 = medium;
    item.friendship_value_3 = high;
  }
};

} // namespace battle
