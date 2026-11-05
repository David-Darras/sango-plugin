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
 * @file trainer_model_extension.h
 * @brief Shows the 3D models of the trainers of Pokémon X and Y in the battles
 *        of ORAS.
 */

#pragma once

#include "common.h"
#include "battle/constant/trainer.h"
#include "battle/constant/trainer_model.h"
#include "battle/constant/xy_trainer_model.h"
#include "battle/native/config.h"
#include "battle/native/trainer_model_manager.h"
#include "core/fixed_list.h"

namespace battle {
/**
 * @brief Gives a 3D model of Pokémon X and Y to a trainer of the game.
 *
 * The game shows a 3D model for some trainers only (the champion, the leaders
 * of the teams, the friends). The table of the models has 15 places and the
 * battle uses the number of the place for its camera and its entrance. So a
 * new model needs a place.
 *
 * The extension borrows a place (the carrier) during the battle of a trainer
 * that has a model: it writes the files of the model of X and Y in the table
 * of the game, and it makes the game choose this place for the trainer. At the
 * start of the next battle, it puts the files of the place back. The camera and
 * the entrance are the ones of the carrier (Trevor, the place of a friend).
 *
 * The files of the models are in sdmc:/sango/trainer_model. The tool
 * tools/export_xy_trainer_models.py writes them. Without the files, the
 * battle shows no model for the trainer.
 *
 * The introduction of the battle is the introduction of the trainer in
 * Pokémon X and Y, with its background and its camera. The extension makes the
 * game read the files of X and Y instead of the files of the introduction of
 * the carrier (the sequence and the camera of the defeat), during the battle.
 * Without the files of sequence and layout (sango/trainer_model/sequence and
 * layout), the battle uses the introduction of the carrier.
 *
 * A battle against two trainers (a multi battle) shows the model for both.
 *
 * @code
 * TrainerModelExtension::GetInstance().Assign(TrainerId::kRoute102YoungsterCalvin,
 *                                             XyTrainerModelId::kLysandre);
 * @endcode
 */
class TrainerModelExtension {
  MAKE_SINGLETON(TrainerModelExtension)
public:
  /// The number of the first file of the models of X and Y in the archive of
  /// the trainer models. The 75 files before are the models of the game.
  static constexpr u16 kFirstFile = 75;
  /// The number of files of a model: the model, the base and 3 motions.
  static constexpr u16 kFilesPerModel = 5;
  /// The number of trainers that can have a model.
  static constexpr u32 kMaxAssignments = 64;

  /// false: the extension does nothing. The battles are the battles of the
  /// game.
  bool is_enabled = true;

  /// The number of choices of player_model.
  static constexpr u8 kPlayerModelCount =
      1 + static_cast<u8>(TrainerModelId::kCount) +
      static_cast<u8>(XyTrainerModelId::kCount);

  /**
   * @brief The 3D model that the battle shows for the player (ORAS only).
   *
   * 0: the model of the game (May or Brendan). 1 to 15: a model of ORAS, in the
   * order of TrainerModelId. 16 to 28: a model of X and Y, in the order of
   * XyTrainerModelId (it needs the files of sango/trainer_model). The change
   * applies at the next battle. The animations come from the model: a model
   * that has no throwing motion shows the Pokémon without the throw.
   */
  u8 player_model = 1 + static_cast<u8>(TrainerModelId::kCount) +
                    static_cast<u8>(XyTrainerModelId::kLysandre);

  /// true: the player has a 2D picture, the picture of a class (the
  /// introduction of the game for the trainers with a 2D picture), and no 3D
  /// model. The change applies at the next battle.
  bool use_player_picture = false;
  /// The class that gives the picture of the player.
  u16 player_picture_class = 0;

  /// The number of the file of the 2D introduction of the player.
  static constexpr u32 kPictureSequenceFile = 1350;
  /// Gives the class to the file of the 2D introduction of the player that
  /// the archive layer just read (the placeholder in the file becomes the
  /// class). Does nothing for another file.
  static void PatchPlayerPicture(u32 file_id, void* data, u32 size);
  /// Hides the 3D model of the player in the entry of the Pokémon when the
  /// player has a picture. The battle hook of the entry calls it.
  static void OnEntryAnimation(uptr scene, u32* trainer_models);

  static void Initialize();

  /// Looks for the place of the targets of the trainers in the battle scene,
  /// each time that the game starts the introduction of a trainer. The
  /// game_extension calls it.
  static void OnPlaySequence(uptr scene, u32 id, bool is_move);

  /// Gives the model to the trainer. Returns false when the list is full.
  /// A second call for the same trainer changes its model.
  bool Assign(TrainerId trainer, XyTrainerModelId model);

  /// Removes the model of the trainer.
  void Remove(TrainerId trainer);

  /// Removes the models of all the trainers.
  void Clear();

  /// Returns the files of a model of X and Y in the archive.
  static TrainerModelFiles GetFiles(XyTrainerModelId model);

  /// The number of the first new file of the sequences, in the archive of the
  /// battle animations. The sequence of a model has the number
  /// kFirstSequenceFile + the number of the model.
  static constexpr u32 kFirstSequenceFile = 1300;
  /// The number of the first sequence for the player. It is the sequence of the
  /// model, far from the stage, so the Pokémon of the enemy do not show.
  static constexpr u32 kFirstPlayerSequenceFile = 1320;
  /// The same for the introductions of the models of ORAS: the file 1340 is the
  /// copy of the introduction 793 of the game, and so on to 1349.
  static constexpr u32 kFirstOrasPlayerSequenceFile = 1340;
  /// The number of the first file of the packs of battle motions of the models
  /// of ORAS, with the motions that the battle asks for. The pack of the model
  /// n is the file kFirstOrasMotionFile + n.
  static constexpr u16 kFirstOrasMotionFile = 140;
  /// The number of the new file of the layout of the background, in the
  /// archive of the layouts of the introductions.
  static constexpr u32 kLayoutFile = 41;
  /// The models that trainers.txt gives to a trainer: the values 0 to 12 are
  /// the models of X and Y (XyTrainerModelId), the values 13 to 27 are the
  /// models of the game (TrainerModelId + 13).
  static constexpr u8 kFirstGameModel = 13;

private:
  struct Assignment {
    TrainerId trainer;
    XyTrainerModelId model;
  };

  static void OnTrainerBattle(Config& config, TrainerId& trainer);
  static void OnWildBattle(Config& config);
  static s32 GetTrainerClassModelHook(void* trainer_class);
  static s32 GetTrainerClassModelEntranceHook(void* trainer_class);
  static bool StartSideEntryHook(uptr scene, bool is_player, bool is_fade);

  // Plays the introduction of the model of X and Y for the player, before
  // the player throws the Poké Ball.
  void StartPlayerIntroduction(uptr scene);
  // The new file of the introduction of the model of the player, or false
  // when the model has none (the model of the game).
  bool GetPlayerSequence(u32* sequence) const;

  // Chooses the model of the battle that starts.
  void Select(TrainerId trainer);
  // Shows the model of player_model for the player.
  void ApplyPlayerModel();
  // Puts the models of the player back, as the game has them.
  void RestorePlayerModel();
  // Shows a model of the game for the trainer of the battle.
  void SelectGameModel(u32 model);
  // The number of the introduction that the game has for a model of the game
  // (a number of TrainerModelId), or 0.
  static u32 GetGameIntroduction(u32 model);
  // Gives the files of the model player_model. Returns false when a file of
  // X and Y is not on the SD card.
  bool GetPlayerFiles(TrainerModelFiles* files) const;
  // Puts the files of the carrier back, as the game has them.
  void Restore();

  core::FixedList<Assignment, kMaxAssignments> assignments_;
  /// The model of the battle, or -1 when the trainer has no model.
  s32 selected_ = -1;
  /// The place that the battle uses to show the model of X and Y. It has the
  /// introduction of a friend, which the extension replaces.
  static constexpr TrainerModelId kCarrier = TrainerModelId::kTrevor;
  /// The files of the introduction of the carrier: the sequence, and the camera
  /// of the defeat.
  static constexpr u32 kCarrierSequenceFile = 794;
  static constexpr u32 kCarrierLoseCameraFile = 800;

  /// The files of the table before the extension changed them.
  TrainerModelFiles original_ = {};
  bool is_changed_ = false;
  /// true when the extension replaced the files of the introduction.
  bool is_introduction_replaced_ = false;
  /// The new file of the sequence that replaces the one of the carrier.
  u32 enemy_sequence_ = 0;

  enum class PlayerIntroduction : u8 { kIdle, kPlaying, kDone };
  PlayerIntroduction player_introduction_ = PlayerIntroduction::kIdle;
  /// The place of the two targets of the trainers (two numbers) in the battle
  /// scene, or 0 when it is not known.
  u32 target_offset_ = GAME_CONSTANT(0, 0x6D4);
  /// The files of the places of May and Brendan before the change.
  TrainerModelFiles player_original_[2] = {};
  bool is_player_changed_ = false;
};
} // namespace battle
