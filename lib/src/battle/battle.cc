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
 * @file battle.cc
 * @brief Changes the battles: animations, health bars, capture rules, Mega Evolutions.
 *
 * The declarations are in battle/patch/battle.h.
 */

#include "battle/patch/battle.h"

#include "battle/native/manager.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/type_chart.h"
#include "core/hook.h"
#include "overworld/patch/overworld.h"
#include "system/native/sound.h"
#include "pokemon/address.h"

namespace battle {

namespace {
core::Hook<bool(void*, Team*, LevelUpData*)> level_up_hook;
core::Hook<void(void*, u8, bool)> start_mega_evolution_animation_hook;
core::Hook<void(void*, EntryAnimationData*)> start_entry_animation_hook;
core::Hook<void(uptr, u32, u8)> start_background_music_hook;
core::Hook<void(uptr, u16)> play_animation_hook;
core::Hook<void(uptr)> update_view_hook;
core::Hook<void(uptr, uptr, uptr)> pokemon_model_settings_hook;
core::Hook<bool(u32, u32, u32, u32, u32, u32)> check_pokemon_captured_hook;
core::Hook<void(uptr, u16, u32)> update_gauge_hook;
} // namespace

void Battle::Initialize() {
  level_up_hook.Install(address::kLevelUp, LevelUpHook, address::kVtable);
  start_mega_evolution_animation_hook.Install(
      address::kStartMegaEvolutionAnimation, StartMegaEvolutionAnimationHook,
      address::kVtable);
  start_entry_animation_hook.Install(address::kStartEntryAnimation,
                                     StartEntryAnimationHook, address::kVtable);
  start_background_music_hook.Install(address::kStartBackgroundMusic,
                                      StartBackgroundMusicHook);
  play_animation_hook.Install(address::kPlayAnimation, PlayAnimationHook,
                              address::kVtable);
  update_view_hook.Install(address::kUpdateView, UpdateViewHook,
                           address::kVtable);
  pokemon_model_settings_hook.Install(pokemon::address::kPokemonModelSettings,
                                      PokemonModelSettingsHook);
  check_pokemon_captured_hook.Install(
      pokemon::address::kBattleCheckPokemonCaptured, CheckPokemonCapturedHook,
      address::kVtable);
  update_gauge_hook.Install(address::kUpdateGauge, UpdateGaugeHook,
                            address::kVtable);
}

void Battle::PatchUpdate() {
  const u32 kMax = 20;
  static u32 counter = 0;
  counter--;
  if (counter != 0) return;
  counter = kMax;

  auto& feat = GetInstance();

  if (feat.inverse_stats) {
    auto& server_team = Manager::GetTeam(true, 0);
    auto& client_team = Manager::GetTeam(false, 0);
    for (u32 i = 0; i < server_team.count; i++) {
      server_team.pokemon[i]->InverseStats();
      client_team.pokemon[i]->InverseStats();
    }
  }

  if (feat.metronome_only) {
    for (u32 j = 0; j < 2; j++) {
      auto& server_team = Manager::GetTeam(true, j);
      auto& client_team = Manager::GetTeam(false, j);
      for (u32 i = 0; i < server_team.count; i++) {
        server_team.pokemon[i]->SetMetronome();
        client_team.pokemon[i]->SetMetronome();
      }
    }
  }

  if (!feat.mega_restriction) {
    Manager::GetInstance().ResetMegaEvolutions();
  }

  if (feat.sync_team_hp) {
    bool kill_all = false;
    auto& server_team = Manager::GetTeam(true, 0);
    auto& client_team = Manager::GetTeam(false, 0);
    for (u32 i = 0; i < server_team.count; i++) {
      auto& pkm = *server_team.pokemon[i];
      if (pkm.hp == 0) {
        kill_all = true;
        break;
      }
    }
    if (kill_all) {
      for (u32 i = 0; i < server_team.count; i++) {
        server_team.pokemon[i]->hp = 0;
        client_team.pokemon[i]->hp = 0;
      }
    }
  }
}

void Battle::PatchLoad() {
  MEMORY_SCOPE(sys::address::kMemoryRegionCro, 0xD8000);
  TypeChart::PatchLoad();

  auto& feat = GetInstance();
  if (!feat.can_use_item) {
    WRITE8(address::kMenuEntryHpPp, 2);
    WRITE8(address::kMenuEntryStatus, 2);
    WRITE8(address::kMenuEntryBattle, 2);
  }

  if (feat.same_ratio_for_all_pokeball) {
    ARM_NOP(address::kMasterBallCheck);
    ARM_NO_COND(address::kMasterBallBranch);

    WRITE32(address::kBallCatchRate, 0xE3A00A01); // mov r0, #0x1000
    ARM_RET(address::kBallCatchRate + 4);
  }

  if (feat.no_shader) {
    ARM_RET(renderer::address::kApplyShader);
    ARM_RET(renderer::address::kApplyShader2);
  }

  if (!feat.mega_restriction) {
    ARM_NOP(address::kMegaRestrictionCheck);
    ARM_NOP(address::kMegaRestrictionCheck2);

    if (feat.unlimited_mega_evolution) {
      WRITE32(address::kCanMegaEvolved, 0xE3A00001);
      ARM_RET(address::kCanMegaEvolved + 4);
    }
  }
}

bool Battle::CheckPokemonCapturedHook(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4,
                                      u32 p5) {
  auto& feat = GetInstance();
  u8* battle_type = (u8*)(READ32(READ32(p1 + 4) + 0x10));
  const u8 original_type = *battle_type;
  if (feat.is_capture_allowed != nullptr && !feat.is_capture_allowed()) {
    *battle_type = 1;
  }
  bool result = check_pokemon_captured_hook(p0, p1, p2, p3, p4, p5);
  if (result) {
    if (feat.on_captured != nullptr) feat.on_captured();
  } else {
    *battle_type = original_type;
  }

  return result;
}

Color8 Battle::LerpColor(Color8 a, Color8 b, f32 t) {
  Color8 c;
  c.r = a.r + (u8)((f32)(b.r - a.r) * t);
  c.g = a.g + (u8)((f32)(b.g - a.g) * t);
  c.b = a.b + (u8)((f32)(b.b - a.b) * t);
  c.a = 255;
  return c;
}

Color8 Battle::GetHpGaugeColor(f32 ratio) {
  if (ratio < 0.0f) ratio = 0.0f;
  if (ratio > 1.0f) ratio = 1.0f;

  Color8 magenta = {255, 0, 180, 255};
  Color8 red = {255, 0, 60, 255};
  Color8 orange = {255, 100, 0, 255};
  Color8 yellow = {255, 230, 0, 255};
  Color8 green = {0, 255, 90, 255};
  Color8 cyan = {0, 230, 255, 255};

  if (ratio < 0.2f) {
    return LerpColor(magenta, red, ratio / 0.2f);
  } else if (ratio < 0.4f) {
    return LerpColor(red, orange, (ratio - 0.2f) / 0.2f);
  } else if (ratio < 0.6f) {
    return LerpColor(orange, yellow, (ratio - 0.4f) / 0.2f);
  } else if (ratio < 0.8f) {
    return LerpColor(yellow, green, (ratio - 0.6f) / 0.2f);
  } else {
    return LerpColor(green, cyan, (ratio - 0.8f) / 0.2f);
  }
}

void Battle::UpdateGaugeHook(uptr gauge, u16 max_hp, u32 new_hp) {
  update_gauge_hook(gauge, max_hp, new_hp);
  uptr res = ((uptr(*)(uptr))address::kGetHpGaugePane)(
      READ32(gauge + 48));

  f32 ratio = (max_hp > 0) ? (f32)new_hp / (f32)max_hp : 0.0f;
  Color8 color = GetHpGaugeColor(ratio);

  WRITE32(res + 16, color.GetRaw());
}

void Battle::UpdateViewHook(uptr self) {
  if (kCameraOffset != 0) {
    u32* camera = *(u32**)(self + kCameraOffset);
    camera[4] = 0; ///< Do not use the split view.
    camera[5] = 0x7FFFFFFF; ///< Disable the camera animation.
  }

  update_view_hook(self);
}

void Battle::PokemonModelSettingsHook(uptr model, uptr p0, uptr p1) {
  pokemon_model_settings_hook(model, p0, p1);
  if (!GetInstance().fix_pokemon_size) return;

  WRITE32(model + kModelShownHeightOffset, READ32(model + kModelRealHeightOffset));
  WRITE32(model + kModelShownScaleOffset, 0x3F800000);
}

void Battle::PlayAnimationHook(uptr view_manager, u16 id) {
  if (id == kShinyAnimationId && !GetInstance().show_shiny_animation) return;
  return play_animation_hook(view_manager, id);
}

void Battle::StartBackgroundMusicHook(uptr sound_manager, u32 id, u8 p2) {
  if (GetInstance().sync_overworld_music) {
    id = sys::Sound::kBankBackgroundMusic +
         static_cast<u32>(overworld::Overworld::GetInstance().background_music);
  }
  return start_background_music_hook(sound_manager, id, p2);
}

void Battle::StartEntryAnimationHook(void* p0, EntryAnimationData* data) {
  auto& config = GetInstance();
  data->skip_pokeball_animation = !config.show_pokeball_animation;
  data->is_long_encounter_animation = config.is_long_encounter_animation;
  data->use_trainer_pov = !config.show_enemy_pov;
  data->show_fade_in = config.show_fade_in;
  data->skip_pokeball_animation = !config.show_pokeball_animation;
  data->show_shiny_animation = config.show_shiny_animation;
  if (kHasTrainerVisibilityFlag)
    data->dont_show_trainer = !config.show_trainer_animation;

  start_entry_animation_hook(p0, data);
}

void Battle::StartMegaEvolutionAnimationHook(void* view, u8 target,
                                             bool is_long_animation) {
  start_mega_evolution_animation_hook(
      view, target, GetInstance().is_long_mega_evolve_animation);
}

bool Battle::LevelUpHook(void* self, Team* team,
                         LevelUpData* data) {
  return level_up_hook(self, team, data);
}
} // namespace battle