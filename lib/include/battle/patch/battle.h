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
 * @file battle.h
 * @brief Changes the battles: animations, health bars, capture rules, Mega Evolutions.
 */

#pragma once

#include <type_traits>

#include "common.h"
#include "battle/native/entry_animation_data.h"
#include "battle/native/level_up_data.h"
#include "battle/native/team.h"

namespace battle {

/// The settings of battle::Battle.
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
  bool fix_pokemon_size = true; ///< Shows the Pokémon with their real sizes.
  bool sync_overworld_music = false; ///< Keeps the overworld music in battle.
  bool sync_team_hp = false; ///< When one Pokémon of the first team faints, all the Pokémon of this team faint.
  bool inverse_stats = false; ///< Exchanges the physical and the special stats.
  bool metronome_only = false; ///< All the moves are Metronome.
  bool show_type_helper = false; ///< Shows the effectiveness of the moves.

  bool mega_restriction = true; ///< true: the rule of the game (one Mega Evolution in each battle). false: no limit.
  bool unlimited_mega_evolution = true; ///< When mega_restriction is false: the game always accepts a Mega Evolution.
};
static_assert(std::is_standard_layout<BattleSettings>::value,
              "BattleSettings must have standard layout");

/// Changes the battles. A product sets the settings and the callbacks.
struct Battle : public BattleSettings {
  MAKE_SINGLETON(Battle)
public:
  /// Returns false to stop the capture of the wild Pokémon.
  typedef bool (*CaptureAllowedCallback)();
  /// Called after the player catches a Pokémon.
  typedef void (*CapturedCallback)();

  CaptureAllowedCallback is_capture_allowed = nullptr;
  CapturedCallback on_captured = nullptr;

  static void Initialize();
  /// Called at each frame of a battle.
  static void PatchUpdate();
  /// Called when a battle starts. Enables the hooks of the battle CRO.
  static void PatchLoad();

private:
  /// The animation of a shiny Pokémon that enters the battle.
  static constexpr u16 kShinyAnimationId = 621;
  static constexpr u32 kCameraOffset = GAME_CONSTANT(0, 408);
  static constexpr u32 kModelRealHeightOffset = 0x274;
  static constexpr u32 kModelShownHeightOffset = 0x278;  static constexpr u32 kModelShownScaleOffset = GAME_CONSTANT(0x3E8, 0x3F0);
  static constexpr bool kHasTrainerVisibilityFlag = GAME_CONSTANT(false, true);

  static bool CheckPokemonCapturedHook(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4,
                                       u32 p5);
  static Color8 LerpColor(Color8 a, Color8 b, f32 t);
  static Color8 GetHpGaugeColor(f32 ratio);
  static void UpdateGaugeHook(uptr gauge, u16 max_hp, u32 new_hp);
  static void UpdateViewHook(uptr self);
  static void PokemonModelSettingsHook(uptr model, uptr p0, uptr p1);
  static void PlayAnimationHook(uptr view_manager, u16 id);
  static void StartBackgroundMusicHook(uptr sound_manager, u32 id, u8 p2);

  static void StartEntryAnimationHook(void* p0, EntryAnimationData* data);
  static void StartMegaEvolutionAnimationHook(void* view, u8 target,
                                              bool is_long_animation);
  static bool LevelUpHook(void* self, Team* team, LevelUpData* data);
};
} // namespace battle
