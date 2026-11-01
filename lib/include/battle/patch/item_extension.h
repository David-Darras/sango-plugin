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
 * @file item_extension.h
 * @brief Adds new items to the game: a name, a description, an icon and the
 *        reactions of a held item in battle.
 *
 * @see docs/tutorials/15-add-an-item.md
 */

#pragma once

#include "common.h"
#include "core/fixed_list.h"
#include "battle/native/reaction_table.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/species.h"

namespace pokemon {
struct ItemData;
struct CoreData;
struct EvolutionData;
}

namespace battle {
struct Pokemon;

/**
 * @brief The description of a new item, or of an item of the game that changes.
 *
 * With an id that the game does not use (from ItemExtension::kFirstId to
 * ItemExtension::kLastId), the description makes a new item. The game loads the
 * data of a Heart Scale for a new item. Then @ref patch_data changes this data.
 *
 * With the id of an item of the game (1 to 775), the description changes this
 * item. A field that you leave empty (null, ItemId::kNone) keeps the value of
 * the game.
 */
struct ItemSpec {
  /// The id of the new item.
  ItemId id;
  /// The name that the game shows. Use u"..." text. Null keeps the name of
  /// an item of the game.
  const c16* name;
  /// The description that the game shows. Use "\n" for a new line. Null keeps
  /// the description of an item of the game.
  const c16* description;
  /// The item of the game whose icon the new item shows when it has no icon
  /// of its own (for example ItemId::kLeftovers). ItemId::kNone keeps the icon
  /// of an item of the game.
  ///
  /// The icon of its own is the file `items/<name>.bclim` of the data folder
  /// (sdmc:/sango for the overlay, see core/data_folder.h), with the
  /// same letters as the name (for example `Plasma Core.bclim`). The image has
  /// 30 x 30 pixels (see tools/png_to_item_icon.py).
  ItemId icon_item;
  /// Sets the data of the item (price, pocket, Fling...). Can be null.
  void (*patch_data)(pokemon::ItemData& item);
  /// The reactions of the item when a Pokémon holds it in a battle: what it
  /// does, and when. Can be null. For an item of the game, null keeps the
  /// reactions of the game, and a table replaces all of them.
  const ReactionTable* reactions;
  /// The number of entries in @ref reactions.
  u32 reaction_count;
  /// Makes the item usable in the Bag on a Pokémon. The player chooses "Use"
  /// and a Pokémon, and the plugin calls this function to know if the item can
  /// act on it. Null means that the item always can. It only matters when
  /// @ref use is not null. The data of the Pokémon is readable.
  bool (*can_use)(const pokemon::CoreData& pokemon);
  /// What the item does when the player uses it on a Pokémon in the Bag. Null
  /// means that the player cannot use the item in the Bag. The function
  /// changes the data of the Pokémon (shiny state, ability, species...) and
  /// returns the message that the Bag shows, or null for a message of the
  /// plugin. The text can have `{pokemon}`: it becomes the name of the
  /// Pokémon. The game then removes one item and updates the screen.
  const c16* (*use)(pokemon::CoreData& pokemon);
  /// Makes the item an evolution stone: the player uses it in the Bag, and the
  /// Pokémon evolves with the animation of the game. The plugin calls this
  /// function when the game loads the evolutions of a species. It gets the
  /// species and its evolutions, and returns the species that the item gives,
  /// or SpeciesId::kNone for a species that does not evolve with the item.
  /// Null means that the item is not an evolution stone.
  SpeciesId (*evolve_into)(SpeciesId species,
                           const pokemon::EvolutionData& evolutions);
  /// true: the @ref use function changes the HP with ItemExtension::ChangeHp().
  /// The Bag then plays the animation of the HP bar, like a medicine: the bar
  /// goes down when the HP go down.
  bool changes_hp;
};

/**
 * @brief Adds new items to the game (ORAS only).
 *
 * The library calls Initialize() at start-up. Your product calls AddItem().
 * The item has its name and its description in the summary and in the menus,
 * and its icon in the party menu and in the summary. A Pokémon that holds the
 * item gets its reactions in a battle.
 *
 * @code
 * static const battle::ReactionTable kMyItemReactions[] = {
 *     {battle::MomentKind::kPokemonEntered, MyItemReaction},
 * };
 *
 * battle::ItemExtension::AddItem({kMyItem, u"My Item", u"A new item.",
 *                                 ItemId::kLeftovers, nullptr,
 *                                 kMyItemReactions, SIZE(kMyItemReactions)});
 * @endcode
 */
class ItemExtension {
  MAKE_SINGLETON(ItemExtension)
public:
  /// The number of items of the game: the ids 1 to 775 are items of the game.
  static constexpr u32 kGameItemCount = 776;
  /// The first id for a new item.
  static constexpr u32 kFirstId = 800;
  /// The last id for a new item.
  static constexpr u32 kLastId = 1023;
  /// The file of the item data that the game reads for a new item: the data
  /// of the template item (a Heart Scale).
  static constexpr u32 kTemplateFile = static_cast<u32>(ItemId::kHeartScale);
  /// The number of the file that the game reads for the icon of the item
  /// `kFirstId`. The next items use the next numbers. The archive of the item
  /// icons has 631 files.
  static constexpr u32 kFirstIconFile = 4000;
  /// The maximum number of new items.
  static constexpr u32 kMaxItems = 128;

  /// Installs the hooks. The library calls this function at start-up.
  static void Initialize();

  /**
   * @brief Adds a new item.
   * @param spec The description of the item. The function keeps a copy.
   * @return false when the list is full, when the id is not an item of the
   *         game and not between kFirstId and kLastId, or when the id is
   *         already in use.
   */
  static bool AddItem(const ItemSpec& spec);

  /// Returns the new item with this id, or null.
  static const ItemSpec* FindItem(ItemId id);

  /// Returns the number of added items.
  static u32 GetCount() { return GetInstance().items_.Count(); }
  /// Returns an added item (index from 0 to GetCount() - 1).
  static const ItemSpec& GetItem(u32 index) {
    return GetInstance().items_[index < kMaxItems ? index : 0];
  }

  /// Writes the path of the icon of an item: items/<name>.bclim in the data
  /// folder.
  /// Returns false when the item has no name or when the path is too long.
  static bool GetIconPath(const ItemSpec& spec, c16* path, u32 capacity);

  /// Returns the file of the icon of an item in the archive of the item
  /// icons, or 0xFFFFFFFF when the item is not a new item.
  static u32 GetIconFile(ItemId id);

  /// Writes the name of a new item. Returns false for another item.
  static bool PatchName(ItemId item, String* output);
  /// Returns the number of the text file that the game reads now. A text file
  /// that the game loads when it needs it has 0 in its Message object: this
  /// function gives the real number.
  static u32 ResolveTextFile(u32 file_id);
  /// Gives the game the data of the new items. The library calls it each frame,
  /// and it works one time: the game must be ready to load the data of an item.
  static void Update();
  /// Lets the battle keep the new items. The library calls it each time the
  /// battle code loads.
  static void PatchBattleLoad();
  /// Adds the evolutions of the evolution stones to the evolutions of a
  /// species. The library calls it each time the game loads them.
  static void PatchEvolution(SpeciesId species, pokemon::EvolutionData& table);
  /// Replaces the message of the Bag with the message of the item that the
  /// player just used. Returns false when it is another message.
  static bool PatchUseMessage(u32 file_id, u32 text_id, String* output);
  /// Writes the description of a new item. Returns false for another item.
  static bool PatchDescription(ItemId item, String* output);
  /// For a `use` function: adds HP to the Pokémon (a positive number) or removes
  /// HP (a negative number). The HP stay between 1 and the maximum HP. The
  /// change happens when the function ends.
  static void ChangeHp(s32 delta) { GetInstance().hp_delta_ += delta; }

private:
  /// The size of the table of the item icons.
  static constexpr u32 kIconTableSize = kLastId + 1;

  static void MakeIconTable();
  static bool RecoverCheckHook(void* pokemon, u16 item, u16 move, void* heap);
  static bool RecoverHook(void* pokemon, u16 item, u16 move, u16 place,
                          void* helper, void* heap);
  static void MakeUsableInBag(pokemon::ItemData& item, const ItemSpec& spec);
  static void EnableHpBarHook();
  static bool HpBarStepHook(void* bag, u32* counter, u32 hp, u32 max_hp,
                            u32 place);
  static u32 GetTextOfFileHook(void* context, u32 text_id, u32 file_id,
                               String* output);
  static void ItemDataHook(pokemon::ItemData* item, ItemId id, void* heap);
  static void ItemNameHook(pokemon::ItemData* item, String* output,
                           void* heap);
  static uptr RegisterItemHook(Pokemon* pkm);
  static void RegisterItemCheckedHook(Pokemon* pkm);
  static uptr RegisterTemporaryItemHook(Pokemon* pkm, ItemId item);

  core::FixedList<ItemSpec, kMaxItems> items_;
  u32 icon_table_[kIconTableSize] = {};
  bool is_icon_table_ready_ = false;
  u32 current_text_file_ = 0;
  u32 icon_file_count_ = 0;
  c16 use_message_[128] = {};
  bool has_use_message_ = false;
  s32 hp_delta_ = 0;
  bool is_data_given_ = false;
};

} // namespace battle
