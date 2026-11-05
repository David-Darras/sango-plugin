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
 * @file maker_reader.h
 * @brief Reads the text files of Sango Maker (encounters.txt, trainers.txt,
 * species.txt, story.txt, characters.txt, rules.txt, shops.txt, items.txt,
 * moves.txt).
 *
 * The reader only reads: it gives each part of a file to a maker::Sink, and
 * the sink applies it. core::DataFiles has the sink that changes the game.
 * This file has no game structure, so the tests of tests/ compile it on a
 * computer.
 *
 * @code
 * class CountTrainers : public core::maker::Sink {
 *  public:
 *   void OnTrainer(TrainerId) override { count++; }
 *   u32 count = 0;
 * };
 * CountTrainers sink;
 * core::maker::ReadTrainers(text, size, sink);
 * @endcode
 *
 * @see docs/tutorials/18-make-a-rom-hack-without-code.md
 */

#pragma once

#include "core/types.h"
#include "battle/constant/ai.h"
#include "battle/constant/format.h"
#include "battle/constant/trainer.h"
#include "battle/constant/trainer_class.h"
#include "battle/constant/xy_trainer_model.h"
#include "overworld/constant/encounter_method.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/patch/encounter_slot.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/move_data_kind.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/shiny_rate.h"
#include "pokemon/constant/species.h"
#include "pokemon/constant/type.h"
#include "pokemon/native/shop_item.h"
#include "pokemon/patch/level_move.h"
#include "pokemon/patch/shop_pokemon.h"
#include "script/constant/script.h"

namespace core {

/// The kinds of names of the data files.
enum class NameKind : u8 {
  kSpecies,
  kMove,
  kItem,
  kAbility,
  kNature,
  kType,
  kForm,
  kTrainer,
  kModel,
  kEvolutionMethod,
  kEncounterMethod,
  kFormat,
  kMap,
  kMusic,
  kTrainerModel,
  kTrainerClass,
};

/// Finds the values of the names of the data files.
class DataNames {
public:
  /**
   * @brief Finds the value of a name ("Thunder Shock") or of a number ("84").
   * @param text The name. It does not need a 0 at the end.
   * @param length The number of bytes of the name.
   * @param value The value.
   * @return false when the name does not exist.
   */
  static bool Find(NameKind kind, const c8* text, u32 length, u32* value);

  /**
   * @brief Finds the name of a value, in the form of the files: lowercase,
   *        without spaces and signs ("thundershock"). Find() reads it.
   * @return The name, or null when the value has no name.
   */
  static const c8* GetName(NameKind kind, u32 value);

  /**
   * @brief Gives back the memory of the tables of names. The console plugin
   *        reads a table from the SD card (sango/names) when it needs it and
   *        keeps it: call this function when the reading of the files ends.
   *        The tests of the computer have the tables in the program: the
   *        function does nothing.
   */
  static void Release();

  /// Returns true when the tables of names are available (the files of
  /// sango/names are on the SD card).
  static bool IsAvailable();
};

namespace maker {

/// A Pokémon of a trainer of trainers.txt. A part that the line does not give
/// has its `has_...` value false: the game value stays.
struct Opponent {
  SpeciesId species;
  u8 level;
  ItemId item;
  AbilityId ability;
  Nature nature;
  FormId form;
  u8 ivs;
  u8 evs[6]; ///< HP, Attack, Defense, Sp. Atk, Sp. Def, Speed.
  MoveId moves[4];
  bool has_level;
  bool has_item;
  bool has_ability;
  bool has_nature;
  bool has_form;
  bool has_ivs;
  bool has_evs;
  bool has_moves;
  bool is_shiny;
};

/// The changes of an item of items.txt. A part that the file does not give
/// has its `has_...` value false: the game value stays.
struct ItemChange {
  ItemId item;
  u16 price;
  u8 fling_power;
  u8 hp_restore;
  s8 evs[6]; ///< HP, Attack, Defense, Sp. Atk, Sp. Def, Speed.
  bool has_price;
  bool has_fling_power;
  bool has_hp_restore;
  bool has_evs;
};

/// The changes of a move of moves.txt. A part that the file does not give
/// has its `has_...` value false: the game value stays.
struct MoveChange {
  MoveId move;
  TypeId type;
  DamageCategory category;
  u8 power;
  u8 accuracy; ///< 101: the move never misses.
  u8 pp;
  s8 priority;
  bool has_type;
  bool has_category;
  bool has_power;
  bool has_accuracy;
  bool has_pp;
  bool has_priority;
};

/// The shop id of "[all shops]" in shops.txt (pokemon::CustomShop::kAnyShop).
constexpr u32 kAllShops = 0xFFFFFFFF;
/// The number of items or Pokémon of a shop at most.
constexpr u32 kMaxShopItems = 60;

/// A character or a trainer of characters.txt.
struct Character {
  MapId map;
  ModelId model;
  u16 tile_x;
  u16 tile_z;
  u16 height;
  u8 facing; ///< 0 up, 1 down, 2 left, 3 right.
  ScriptId script; ///< ScriptId::kNone for a trainer.
  TrainerId trainer; ///< TrainerId::kNone for a character with a script.
  u8 sight; ///< The tiles that a trainer sees.
};

/// The commands of the scripts of story.txt.
enum class Op : u8 {
  kSay, kSign, kAsk, kChoose, kIf, kElse, kEnd, kStop, kFacePlayer,
  kGiveItem, kTakeItem, kGivePokemon, kGiveMoney, kTakeMoney,
  kSetFlag, kClearFlag, kSetVariable, kAddVariable,
  kBattleTrainer, kBattleWild, kWarp, kHeal, kFadeOut, kFadeIn,
  kMusic, kMusicEnd, kWait, kSound, kCallScript,
};

/// The conditions of "if".
enum class Condition : u8 {
  kNone, kYes, kNo, kChoice, kFlag, kNotFlag,
  kVarEqual, kVarNotEqual, kVarAtLeast, kVarAtMost, kVarMore, kVarLess,
  kHasItem, kMoney, kWon, kLost,
};

/// One command of a script. The meaning of `a`, `b`, `c` and `d` depends on
/// the command.
struct Instruction {
  Op op;
  Condition condition;
  u8 extra; ///< The direction of a warp, or the colour of a fade.
  u16 a;
  u16 b;
  u16 c;
  u16 d;
  u16 jump; ///< The next instruction of "if" (when false) and of "else".
  u16 line; ///< The line of the command in story.txt.
  const c16* text; ///< The text (the choices of "choose" follow each other).
};

/// A script of story.txt: its commands are GetInstruction(first) to
/// GetInstruction(first + count - 1).
struct Story {
  u16 id;
  u16 first;
  u16 count;
};

/// The rules of rules.txt that are on or off. The key of each rule is its
/// name with spaces: kItemsInBattle is "items in battle".
enum class Rule : u8 {
  kItemsInBattle,    ///< The player uses items in battle.
  kOneMegaEvolution, ///< One Mega Evolution in each battle.
  kSameCatchRate,    ///< All the Poké Balls have the same catch rate.
  kInverseStats,     ///< The physical and the special stats change places.
  kMetronomeOnly,    ///< All the moves are Metronome.
  kTypeHelper,       ///< The battle shows the effectiveness of the moves.
  kRandomGifts,      ///< The gift Pokémon get a random species.
  kNoEvLimit,        ///< The items have no EV limit.
};

/// The multipliers of rules.txt ("experience", "catch rate", "money").
enum class Multiplier : u8 {
  kExperience, ///< The experience after a battle.
  kCatchRate,  ///< The catch rate of the Poké Balls.
  kMoney,      ///< The money after a battle.
};

/**
 * @brief Receives the parts that the reader reads. Override the functions
 *        that you need: the others do nothing.
 */
class Sink {
public:
  /// A mistake in a file: "story.txt 12: unknown command: jump".
  virtual void OnProblem(const c8* file, u32 line, const c8* what,
                         const c8* word) {}

  // encounters.txt. The slots stay in memory.
  virtual void OnEncounterTable(MapId map, EncounterMethod method,
                                const overworld::EncounterSlot* slots,
                                u32 count) {}

  // trainers.txt: OnTrainer() starts a trainer, the next functions change it,
  // and OnTrainerEnd() ends it. The name stays in memory.
  virtual void OnTrainer(TrainerId trainer) {}
  // [new Name]: gives the number of a free trainer, or kNone when there is
  // none. OnTrainer() with this number follows at once.
  virtual TrainerId OnNewTrainer(const c16* name) { return TrainerId::kNone; }
  // [new class Name, Lass]: makes a class with the data of another class.
  // Returns TrainerClassId::kCount when there is none free. The next
  // functions change the class.
  virtual TrainerClassId OnNewClass(const c16* title, TrainerClassId from) {
    return TrainerClassId::kCount;
  }
  virtual void OnClassTitle(TrainerClassId id, const c16* title) {}
  virtual void OnClassOverworld(TrainerClassId id, ModelId model) {}
  virtual void OnClassPicture(TrainerClassId id, TrainerClassId from) {}
  virtual void OnClassIntroduction(TrainerClassId id, TrainerClassId from) {}
  virtual void OnClassMusic(TrainerClassId id, TrainerClassId from) {}
  virtual void OnTrainerFormat(Format format) {}
  virtual void OnTrainerName(const c16* name) {}
  virtual void OnTrainerMoney(u8 money) {}
  virtual void OnTrainerAi(AiFlags flags) {}
  virtual void OnTrainerModel(XyTrainerModelId model) {}
  virtual void OnTrainerClass(TrainerClassId trainer_class) {}
  virtual void OnTrainerItems(const ItemId* items, u32 count) {}
  virtual void OnTrainerPokemon(const Opponent& pokemon) {}
  virtual void OnTrainerEnd() {}

  // species.txt. The learnsets and the evolutions stay in memory.

  /// A new species: [new Sangoat, Skiddo, Gogoat]. `name` has 12 letters at
  /// most, and the text does not stay in memory. Returns the id of the new
  /// species, or SpeciesId::kNone when the game cannot add it. The other
  /// files then find the species with its name.
  virtual SpeciesId OnNewSpecies(const c16* name, SpeciesId data_from,
                                 SpeciesId model) {
    return SpeciesId::kNone;
  }
  virtual void OnSpeciesStats(SpeciesId species, FormId form,
                              const u8* stats) {}
  virtual void OnSpeciesTypes(SpeciesId species, FormId form, TypeId first,
                              TypeId second) {}
  virtual void OnSpeciesAbilities(SpeciesId species, FormId form,
                                  AbilityId first, AbilityId second,
                                  AbilityId hidden) {}
  virtual void OnSpeciesCatchRate(SpeciesId species, FormId form, u8 rate) {}
  virtual void OnSpeciesMachine(SpeciesId species, FormId form, u8 number,
                                bool is_hm, bool can_learn) {}
  virtual void OnSpeciesLearnset(SpeciesId species, FormId form,
                                 const pokemon::LevelMove* moves, u32 count) {}
  virtual void OnSpeciesEvolutions(SpeciesId species,
                                   const pokemon::EvolutionStep* steps,
                                   u32 count) {}

  // story.txt: a new script. Its commands are in GetInstruction().
  virtual void OnStory(u16 id) {}

  // characters.txt. Returns false when the list of characters is full.
  virtual bool OnCharacter(const Character& character) { return true; }

  // rules.txt. A multiplier is in hundredths: 150 is 1.5.
  virtual void OnRule(Rule rule, bool is_on) {}
  virtual void OnMultiplier(Multiplier multiplier, u32 hundredths) {}
  virtual void OnShinyRate(pokemon::ShinyRate rate) {}
  virtual void OnStarters(const SpeciesId* species) {} ///< 3 species.

  // shops.txt. `shop_id` is kAllShops for all the shops. The lists stay in
  // memory.
  virtual void OnShopItems(u32 shop_id, const pokemon::ShopItem* items,
                           u32 count) {}
  virtual void OnShopPokemons(u32 shop_id,
                              const pokemon::ShopPokemon* pokemons,
                              u32 count) {}

  // items.txt.
  virtual void OnItem(const ItemChange& change) {}

  // moves.txt.
  virtual void OnMove(const MoveChange& change) {}

protected:
  ~Sink() {}
};

/// The number of new species of species.txt at most
/// (pokemon::SpeciesTable::kMaxNewSpecies).
constexpr u32 kMaxNewSpecies = 64;
/// The number of new trainers of trainers.txt at most (the free trainers of
/// the game, see battle/constant/trainer_free_slots.h).
constexpr u32 kMaxNewTrainers = 100;
/// The number of new classes of trainers.txt at most.
constexpr u32 kMaxNewClasses = 64;
/// The length of the name of a new species at most.
constexpr u32 kMaxSpeciesNameLength = 12;

/// The size of the largest file, in bytes.
constexpr u32 kMaxFileSize = 32 * 1024;

/// Reads the text of a file (`size` bytes, UTF-8) and gives its parts to
/// `sink`.
void ReadEncounters(const c8* text, u32 size, Sink& sink);
void ReadTrainers(const c8* text, u32 size, Sink& sink);
void ReadSpecies(const c8* text, u32 size, Sink& sink);
void ReadStory(const c8* text, u32 size, Sink& sink);
void ReadCharacters(const c8* text, u32 size, Sink& sink);
void ReadRules(const c8* text, u32 size, Sink& sink);
void ReadShops(const c8* text, u32 size, Sink& sink);
void ReadItems(const c8* text, u32 size, Sink& sink);
void ReadMoves(const c8* text, u32 size, Sink& sink);

/// Returns the last script of story.txt with this id, or null.
const Story* FindStory(u16 id);
/// Returns a command of the scripts of story.txt.
const Instruction& GetInstruction(u32 index);
/// Returns the name of a command, for example "give item".
const c8* GetCommandName(Op op);

/// Returns true when `address` is in the memory of the reader: the slots,
/// the learnsets, the evolutions, the shop lists and the texts that the sink
/// received.
bool IsReaderMemory(const void* address);

/// Forgets all the files that the reader read: the encounters, the texts,
/// the learnsets and the scripts. The memory of the old files is then free
/// for new files. Call it only when nothing uses the old parts any more.
void Clear();

} // namespace maker
} // namespace core
