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
 * @file trainer_model_extension.cc
 * @brief Shows the 3D models of the trainers of Pokémon X and Y in the battles
 *        of ORAS.
 *
 * The declarations are in battle/patch/trainer_model_extension.h.
 */

#include "battle/patch/trainer_model_extension.h"

#include "battle/address.h"
#include "battle/patch/setup.h"
#include "core/constant/archive_id.h"
#include "core/game_function.h"
#include "core/hook.h"
#include "core/patch/archive.h"
#include "ui/log.h"

namespace battle {

namespace {
// The camera of the defeat of each model of X and Y, as a file of the archive
// of the effect models of ORAS.
constexpr u32 kLoseCameraFiles[] = {819, 822, 827, 833, 837, 840, 847,
                                    853, 858, 862, 865, 868, 871};
static_assert(SIZE(kLoseCameraFiles) ==
                  static_cast<u32>(XyTrainerModelId::kCount),
              "one camera for each model of X and Y");

core::Hook<s32(void*)> get_trainer_class_model_hook;
core::Hook<s32(void*)> get_trainer_class_model_entrance_hook;
core::Hook<bool(uptr, bool, bool)> start_side_entry_hook;

// The place of the step in the battle scene, and of the table of the models of
// the trainers (the player first). ORAS only.
constexpr u32 kStepOffset = 0x6C0;
constexpr u32 kTrainerModelsOffset = 0x118;
// The numbers of the places of the trainers in the battle scene.
constexpr u32 kPlayerPlace = 0;
constexpr u32 kPlayerFriendPlace = 2;
// The first introduction of ORAS that has a copy for the player.
constexpr u32 kFirstOrasIntroduction = 793;
} // namespace

void TrainerModelExtension::Initialize() {
  get_trainer_class_model_hook.Install(address::kGetTrainerClassModel,
                                       GetTrainerClassModelHook);
  get_trainer_class_model_entrance_hook.Install(
      address::kGetTrainerClassModelEntrance,
      GetTrainerClassModelEntranceHook);
  // The battle code is loaded only during a battle.
  if (address::kStartSideEntry != 0) {
    start_side_entry_hook.Install(address::kStartSideEntry,
                                  StartSideEntryHook, address::kVtable);
  }
  auto& setup = Setup::GetInstance();
  setup.on_trainer_battle.Subscribe(OnTrainerBattle);
  setup.on_wild_battle.Subscribe(OnWildBattle);
}

bool TrainerModelExtension::Assign(TrainerId trainer, XyTrainerModelId model) {
  auto& self = GetInstance();
  Assignment* same = self.assignments_.Find(
      [trainer](const Assignment& entry) { return entry.trainer == trainer; });
  if (same != nullptr) {
    same->model = model;
    return true;
  }
  return self.assignments_.Add({trainer, model});
}

void TrainerModelExtension::Remove(TrainerId trainer) {
  GetInstance().assignments_.RemoveIf(
      [trainer](const Assignment& entry) { return entry.trainer == trainer; });
}

void TrainerModelExtension::Clear() { GetInstance().assignments_.Clear(); }

TrainerModelFiles TrainerModelExtension::GetFiles(XyTrainerModelId model) {
  const u16 first = kFirstFile + static_cast<u16>(model) * kFilesPerModel;
  return {first, static_cast<u16>(first + 1), static_cast<u16>(first + 2),
          static_cast<u16>(first + 3), static_cast<u16>(first + 4)};
}

void TrainerModelExtension::OnTrainerBattle(Config& config,
                                            TrainerId& trainer) {
  auto& self = GetInstance();
  self.Select(trainer);
  self.ApplyPlayerModel();
  // The class of the trainer has the introduction with a 2D picture: the 3D
  // introduction shows the model.
  if (self.selected_ >= 0) {
    config.encounter_animation = EncounterAnimationId::kTrainerNormal3d;
  }
}

void TrainerModelExtension::OnWildBattle(Config& /*config*/) {
  auto& self = GetInstance();
  self.Select(TrainerId::kNone);
  self.ApplyPlayerModel();
}

void TrainerModelExtension::Select(TrainerId trainer) {
  Restore();
  if (!is_enabled) return;
  const Assignment* entry = assignments_.Find(
      [trainer](const Assignment& item) { return item.trainer == trainer; });
  if (entry == nullptr) return;

  // A file that is not in the archive and not on the SD card makes the game
  // wait for ever: the battle starts without the model.
  const TrainerModelFiles files = GetFiles(entry->model);
  const u16 numbers[kFilesPerModel] = {files.model, files.base,
                                       files.battle_motions,
                                       files.idle_motions,
                                       files.cinematic_motions};
  for (u16 number : numbers) {
    if (!core::Archive::HasSdFile(core::ArchiveId::kTrainerModel, number)) {
      ui::Log::Print(u"trainer model: no file %u in sango/trainer_model",
                     static_cast<u32>(number));
      return;
    }
  }

  // The carrier shows the model of X and Y until the next battle.
  auto& table = TrainerModelManager::GetInstance();
  original_ = table.GetFiles(kCarrier);
  is_changed_ = true;
  table.SetFiles(kCarrier, files);
  selected_ = static_cast<s32>(entry->model);

  // The introduction of X and Y, with its background and its camera.
  const u32 sequence = kFirstSequenceFile + static_cast<u32>(entry->model);
  if (core::Archive::HasSdFile(core::ArchiveId::kBattleMoveAnimation,
                               sequence) &&
      core::Archive::HasSdFile(core::ArchiveId::kMoveEffectLayout,
                               kLayoutFile)) {
    core::Archive::ReplaceFile(core::ArchiveId::kBattleMoveAnimation,
                               kCarrierSequenceFile, sequence);
    enemy_sequence_ = sequence;
    core::Archive::ReplaceFile(
        core::ArchiveId::kMoveEffectModel, kCarrierLoseCameraFile,
        kLoseCameraFiles[static_cast<u32>(entry->model)]);
    is_introduction_replaced_ = true;
  } else {
    ui::Log::Print(u"trainer model: no sequence in sango/trainer_model");
  }
}

bool TrainerModelExtension::GetPlayerFiles(TrainerModelFiles* files) const {
  const u8 oras_count = static_cast<u8>(TrainerModelId::kCount);
  if (player_model == 0 || player_model >= kPlayerModelCount) return false;
  if (player_model <= oras_count) {
    // A model of the game: its five files are consecutive.
    const u16 first = (player_model - 1) * kFilesPerModel;
    *files = {first, static_cast<u16>(first + 1), static_cast<u16>(first + 2),
              static_cast<u16>(first + 3), static_cast<u16>(first + 4)};
    // The pack of motions of the game has holes (the throw of the player, the
    // Mega Evolution): the SD card can give a pack that fills them.
    const u16 filled = kFirstOrasMotionFile + (player_model - 1);
    if (core::Archive::HasSdFile(core::ArchiveId::kTrainerModel, filled)) {
      files->battle_motions = filled;
    }
    return true;
  }
  *files = GetFiles(static_cast<XyTrainerModelId>(player_model - 1 -
                                                  oras_count));
  const u16 numbers[kFilesPerModel] = {files->model, files->base,
                                       files->battle_motions,
                                       files->idle_motions,
                                       files->cinematic_motions};
  for (u16 number : numbers) {
    if (!core::Archive::HasSdFile(core::ArchiveId::kTrainerModel, number)) {
      return false;
    }
  }
  return true;
}

void TrainerModelExtension::ApplyPlayerModel() {
#ifdef GAME_ORAS
  TrainerModelFiles files = {};
  if (!is_enabled || !GetPlayerFiles(&files)) {
    RestorePlayerModel();
    return;
  }
  // The battle shows May or Brendan for the player: both places get the model.
  auto& table = TrainerModelManager::GetInstance();
  const TrainerModelId slots[2] = {TrainerModelId::kMay,
                                   TrainerModelId::kBrendan};
  if (!is_player_changed_) {
    for (u32 i = 0; i < 2; i++) player_original_[i] = table.GetFiles(slots[i]);
    is_player_changed_ = true;
  }
  for (u32 i = 0; i < 2; i++) table.SetFiles(slots[i], files);
#endif
}

void TrainerModelExtension::RestorePlayerModel() {
#ifdef GAME_ORAS
  if (!is_player_changed_) return;
  auto& table = TrainerModelManager::GetInstance();
  table.SetFiles(TrainerModelId::kMay, player_original_[0]);
  table.SetFiles(TrainerModelId::kBrendan, player_original_[1]);
  is_player_changed_ = false;
#endif
}

void TrainerModelExtension::Restore() {
  if (is_changed_) {
    TrainerModelManager::GetInstance().SetFiles(kCarrier, original_);
  }
  if (is_introduction_replaced_) {
    core::Archive::RemoveReplacement(core::ArchiveId::kBattleMoveAnimation,
                                     kCarrierSequenceFile);
    core::Archive::RemoveReplacement(core::ArchiveId::kMoveEffectModel,
                                     kCarrierLoseCameraFile);
  }
  is_introduction_replaced_ = false;
  enemy_sequence_ = 0;
  player_introduction_ = PlayerIntroduction::kIdle;
  is_changed_ = false;
  selected_ = -1;
}

void TrainerModelExtension::OnPlaySequence(uptr scene, u32 id, bool is_move) {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  // The introductions of the trainers are the files 789 to 819. The game has
  // put the places of the trainers of the far side (1 and 3) in the targets.
  if (is_move || id < 780 || id > 830 || self.target_offset_ != 0) return;
  for (u32 offset = kStepOffset; offset < kStepOffset + 0x100; offset += 4) {
    if (READ32(scene + offset) == 1 && READ32(scene + offset + 4) == 3) {
      self.target_offset_ = offset;
      ui::Log::Print(u"trainer model: targets at +0x%X", offset);
      return;
    }
  }
#else
  (void)scene;
  (void)id;
  (void)is_move;
#endif
}

bool TrainerModelExtension::GetPlayerSequence(u32* sequence) const {
#ifndef GAME_ORAS
  (void)sequence;
  return false;
#else
  const u8 oras_count = static_cast<u8>(TrainerModelId::kCount);
  if (!is_enabled || player_model == 0 || player_model >= kPlayerModelCount) {
    return false;
  }
  if (player_model > oras_count) {
    *sequence = kFirstPlayerSequenceFile + (player_model - oras_count - 1);
    return true;
  }
  // The introduction that the game plays for the model of ORAS (the file 793
  // for the heroes, 794 for the friends...), in the order of TrainerModelId.
  static const u16 kIntroductions[] = {793, 794, 793, 793, 795, 796, 797, 798,
                                       799, 801, 802, 800, 794, 794, 794};
  static_assert(SIZE(kIntroductions) ==
                    static_cast<u32>(TrainerModelId::kCount),
                "one introduction for each model of ORAS");
  *sequence = kFirstOrasPlayerSequenceFile + (kIntroductions[player_model - 1] -
                                              kFirstOrasIntroduction);
  return true;
#endif
}

// The battle scene shows the trainer of a side in this step. The step 0 waits
// for the files, the step 1 waits for the end of the sequence that is playing,
// then shows the model and the Poké Ball. The extension plays its sequence
// when the step 1 starts, and runs the step only when the sequence is
// finished.
bool TrainerModelExtension::StartSideEntryHook(uptr scene, bool is_player,
                                               bool is_fade) {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  if (is_player && self.player_introduction_ == PlayerIntroduction::kPlaying) {
    const bool is_finished = core::GameFunction<bool(uptr, bool)>::Call(
        address::kIsSequenceFinished, scene, false);
    if (!is_finished) return false;
    self.player_introduction_ = PlayerIntroduction::kDone;
    // The sequence of the trainer is the one of the enemy again.
    if (self.enemy_sequence_ != 0) {
      core::Archive::ReplaceFile(core::ArchiveId::kBattleMoveAnimation,
                                 kCarrierSequenceFile, self.enemy_sequence_);
    } else {
      core::Archive::RemoveReplacement(core::ArchiveId::kBattleMoveAnimation,
                                       kCarrierSequenceFile);
    }
  }
  const bool is_start = is_player &&
      self.player_introduction_ == PlayerIntroduction::kIdle &&
      READ32(scene + kStepOffset) == 0;
  const bool result = start_side_entry_hook(scene, is_player, is_fade);
  if (is_start && READ32(scene + kStepOffset) == 1) {
    self.StartPlayerIntroduction(scene);
  }
  return result;
#else
  return start_side_entry_hook(scene, is_player, is_fade);
#endif
}

void TrainerModelExtension::StartPlayerIntroduction(uptr scene) {
#ifdef GAME_ORAS
  u32 sequence = 0;
  if (!GetPlayerSequence(&sequence)) return;
  // The introductions of X and Y have a layout of their own.
  const bool needs_layout = sequence < kFirstOrasPlayerSequenceFile;
  if (!core::Archive::HasSdFile(core::ArchiveId::kBattleMoveAnimation,
                                sequence) ||
      (needs_layout &&
       !core::Archive::HasSdFile(core::ArchiveId::kMoveEffectLayout,
                                 kLayoutFile))) {
    return;
  }
  if (target_offset_ == 0) {
    ui::Log::Print(u"trainer model: no targets, no introduction for the player");
    return;
  }
  const uptr trainer = READ32(scene + kTrainerModelsOffset + 4 * kPlayerPlace);
  if (trainer == 0) return;
  // The commands of the sequence act on the models of the targets: the player
  // takes the place of the trainer of the enemy.
  core::GameFunction<void(uptr)>::Call(address::kAddTrainerModelToScene, trainer);
  WRITE32(scene + target_offset_, kPlayerPlace);
  WRITE32(scene + target_offset_ + 4, kPlayerFriendPlace);
  core::Archive::ReplaceFile(core::ArchiveId::kBattleMoveAnimation,
                             kCarrierSequenceFile, sequence);
  core::GameFunction<void(uptr, u32)>::Call(address::kStartSequence, scene,
                                            kCarrierSequenceFile);
  player_introduction_ = PlayerIntroduction::kPlaying;
#else
  (void)scene;
#endif
}

s32 TrainerModelExtension::GetTrainerClassModelHook(void* trainer_class) {
  const s32 result = get_trainer_class_model_hook(trainer_class);
  auto& self = GetInstance();
  if (self.selected_ < 0) return result;
  return static_cast<s32>(kCarrier);
}

s32 TrainerModelExtension::GetTrainerClassModelEntranceHook(
    void* trainer_class) {
  const s32 result = get_trainer_class_model_entrance_hook(trainer_class);
  // The class of the trainer has no 3D model: the entrance of the first kind.
  if (GetInstance().selected_ >= 0 && result < 0) return 0;
  return result;
}

} // namespace battle
