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
 * species.txt, story.txt, characters.txt).
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
#include "overworld/constant/encounter_method.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/patch/encounter_slot.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/species.h"
#include "pokemon/constant/type.h"
#include "pokemon/patch/level_move.h"
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
  const c16* text; ///< The text (the choices of "choose" follow each other).
};

/// A script of story.txt: its commands are GetInstruction(first) to
/// GetInstruction(first + count - 1).
struct Story {
  u16 id;
  u16 first;
  u16 count;
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
  virtual void OnTrainerFormat(Format format) {}
  virtual void OnTrainerName(const c16* name) {}
  virtual void OnTrainerMoney(u8 money) {}
  virtual void OnTrainerAi(AiFlags flags) {}
  virtual void OnTrainerItems(const ItemId* items, u32 count) {}
  virtual void OnTrainerPokemon(const Opponent& pokemon) {}
  virtual void OnTrainerEnd() {}

  // species.txt. The learnsets and the evolutions stay in memory.
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

protected:
  ~Sink() {}
};

/// The size of the largest file, in bytes.
constexpr u32 kMaxFileSize = 32 * 1024;

/// Reads the text of a file (`size` bytes, UTF-8) and gives its parts to
/// `sink`.
void ReadEncounters(const c8* text, u32 size, Sink& sink);
void ReadTrainers(const c8* text, u32 size, Sink& sink);
void ReadSpecies(const c8* text, u32 size, Sink& sink);
void ReadStory(const c8* text, u32 size, Sink& sink);
void ReadCharacters(const c8* text, u32 size, Sink& sink);

/// Returns the last script of story.txt with this id, or null.
const Story* FindStory(u16 id);
/// Returns a command of the scripts of story.txt.
const Instruction& GetInstruction(u32 index);

/// Forgets all the files that the reader read: the encounters, the texts,
/// the learnsets and the scripts. The memory of the old files is then free
/// for new files. Call it only when nothing uses the old parts any more.
void Clear();

} // namespace maker
} // namespace core
