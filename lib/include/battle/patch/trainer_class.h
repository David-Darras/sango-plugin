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
 * @file trainer_class.h
 * @brief Changes the class of a trainer.
 */

#pragma once

#include "battle/constant/trainer.h"
#include "battle/constant/trainer_class.h"
#include "core/fixed_list.h"
#include "core/singleton.h"
#include "core/types.h"

namespace sys {
struct String;
}

namespace battle {

/**
 * @brief Gives a trainer another class: Youngster, Lass, Team Flare...
 *
 * The class gives the title in the battle ("Youngster Calvin"), the 2D
 * picture and the 3D model of the trainer, its music and its character on the
 * map. The game reads the class in the data of the trainer. The extension
 * changes the class in these data when the game reads them, so everything
 * that depends on the class changes with it.
 *
 * @code
 * battle::TrainerClasses::Set(TrainerId::kRoute102YoungsterCalvin,
 *                             TrainerClassId::kLass);
 * @endcode
 *
 * In Sango Maker, use the key `class` in trainers.txt: `class = Lass`.
 */
class TrainerClasses {
  MAKE_SINGLETON(TrainerClasses)
public:
  /// The number of trainers that can have another class.
  static constexpr u32 kMaxClasses = 128;

  /// Gives the class to the trainer. A second call changes the class.
  /// Returns false when the list is full.
  static bool Set(TrainerId trainer, TrainerClassId trainer_class);

  /// Gives the trainer the class of the game again.
  static void Remove(TrainerId trainer);

  /// Gives all the trainers the class of the game again.
  static void Clear();

  /// Changes the class in the data of the trainer that the game just read.
  /// The archive layer calls it for each file of the trainer data.
  static void Patch(u32 file_id, void* data, u32 size);

  /// The number of new classes at most.
  static constexpr u32 kMaxNewClasses = 64;
  /// The number of letters of the title of a new class at most.
  static constexpr u32 kMaxTitleLength = 20;

  /// Makes a new class with the data of the class `from`, and the title.
  /// The class takes a class that no trainer of the game uses. Returns
  /// TrainerClassId::kCount when there is none.
  static TrainerClassId Define(const c16* title, TrainerClassId from);

  /// Gives the new class its title, its character on the map (a number of
  /// overworld::ModelId), and the pictures, the 3D model and the music of
  /// another class.
  static void SetTitle(TrainerClassId id, const c16* title);
  static void SetOverworld(TrainerClassId id, u32 model);
  static void CopyPicture(TrainerClassId id, TrainerClassId from);
  static void CopyIntroduction(TrainerClassId id, TrainerClassId from);
  static void CopyMusic(TrainerClassId id, TrainerClassId from);

  /// Removes all the new classes.
  static void ClearDefinitions();

  /// Changes the data of a class that the game just read. The archive layer
  /// calls it for each file of the data of the classes.
  static void PatchRecord(u32 file_id, void* data, u32 size);

  /// Gives the title of a new class to the text that the game reads. Returns
  /// true when the class is a new class.
  static bool PatchTitle(u32 class_id, sys::String* output);

private:
  struct Entry {
    TrainerId trainer;
    TrainerClassId trainer_class;
  };

  struct Definition {
    TrainerClassId id;
    u8 record[16];
    c16 title[kMaxTitleLength + 1];
  };

  static Definition* FindDefinition(TrainerClassId id);
  static void SetTitleText(c16* to, const c16* title);

  core::FixedList<Entry, kMaxClasses> entries_;
  core::FixedList<Definition, kMaxNewClasses> definitions_;
  u32 free_count_ = 0;
};

} // namespace battle
