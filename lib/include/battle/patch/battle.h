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

#include <type_traits>

#include "common.h"
#include "battle/native/entry_animation_data.h"
#include "battle/native/level_up_data.h"
#include "battle/native/team.h"

namespace battle {

struct BattleSettings {
  bool is_long_mega_evolve_animation = false;
  bool is_long_encounter_animation = false;
  bool show_enemy_pov = false;
  bool show_trainer_animation = false;
  bool show_pokeball_animation = false;
  bool show_fade_in = false;
  bool show_shiny_animation = false;

  bool no_shader = false;
  bool can_use_item = true;
  bool same_ratio_for_all_pokeball = false;
  bool fix_pokemon_size = true;
  bool sync_overworld_music = false;
  bool sync_team_hp = false;
  bool inverse_stats = false;
  bool metronome_only = false;
  bool show_type_helper = true;

  bool mega_restriction = true;
  bool unlimited_mega_evolution = true;
};
static_assert(std::is_standard_layout<BattleSettings>::value,
              "BattleSettings must have standard layout");

struct Battle : public BattleSettings {
  MAKE_SINGLETON(Battle)
public:
  typedef bool (*CaptureAllowedCallback)();
  typedef void (*CapturedCallback)();

  CaptureAllowedCallback is_capture_allowed = nullptr;
  CapturedCallback on_captured = nullptr;

  static void Initialize();
  static void PatchUpdate();
  static void PatchLoad();

private:
  /// Move id of the animation played when a shiny Pokémon enters the field.
  static constexpr u16 kShinyAnimationId = 621;
  static constexpr u32 kCameraOffset = GAME_CONSTANT(0, 408);
  static constexpr u32 kModelSpeciesOffset = GAME_CONSTANT(0x138, 0x170);
  static constexpr bool kHasTrainerVisibilityFlag = GAME_CONSTANT(false, true);

  static bool CheckPokemonCapturedHook(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4,
                                       u32 p5);
  static Color8 LerpColor(Color8 a, Color8 b, f32 t);
  static Color8 GetHpGaugeColor(f32 ratio);
  static void UpdateGaugeHook(uptr gauge, u16 max_hp, u32 new_hp);
  static void UpdateViewHook(uptr self);
  static void PatchPokemonSize();
  static void PlayAnimationHook(uptr view_manager, u16 id);
  static void StartBackgroundMusicHook(uptr sound_manager, u32 id, u8 p2);

  static void StartEntryAnimationHook(void* p0, EntryAnimationData* data);
  static void StartMegaEvolutionAnimationHook(void* view, u8 target,
                                              bool is_long_animation);
  static bool LevelUpHook(void* self, Team* team, LevelUpData* data);
};
} // namespace battle
