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

#include "common.h"
#include "feature_overworld.h"
#include "hook_manager.h"
#include "game/battle/manager.h"
#include "kaizo.h"

namespace feature {
struct Battle {
  MAKE_SINGLETON(Battle)
public:
  bool is_long_mega_evolve_animation = false;
  bool is_long_encounter_animation = false;
  bool show_enemy_pov = false;
  bool show_trainer_animation = false;
  bool show_pokeball_animation = false;
  bool show_fade_in = false;
  bool show_shiny_animation = false;

  bool no_shader = false;
  bool can_use_item = true;
  bool use_pokeball_boost = true;
  bool fix_pokemon_size = true;
  bool sync_overworld_music = false;
  bool sync_team_hp = false;
  bool inverse_stats = false;

  bool mega_restriction = true;

  struct LevelUpData {
    u32 exp;

    u16 ev_hp;
    u16 ev_attack;

    u16 ev_defense;
    u16 ev_speed;

    u16 ev_special_attack;
    u16 ev_special_defense;

    bool _0;
    bool _1;
    bool use_exp_share;
    bool _2;
  };

  STATIC_INLINE void Initialize() {
    // HookManager::GetInstance().Add(HookID::kOnStartTurn, 0x00759B74,
    //                                (uptr)OnStartTurn);
    //
    //
    // HookManager::GetInstance().Add(HookID::kPlayBattleAnimation, 0x007510A8,
    //                                (uptr)PlayBattleAnimation);
    //
    // HookManager::GetInstance().Add(HookID::kOnLoadTrainerModel, 0x00458214,
    //                                (uptr)OnLoadTrainerModel);
    HookManager::Initialize(HookID::kUpdateExp, ADDRESS_BATTLE_UPDATE_EXP,
                            (uptr)UpdateExpHook, false);
    HookManager::Initialize(HookID::kStartMegaEvolveAnimation,
                            ADDRESS_BATTLE_START_MEGA_EVOLVE_ANIMATION,
                            (uptr)StartMegaEvolveAnimation, false);
    HookManager::Initialize(HookID::kStartBattleAnimation,
                            ADDRESS_BATTLE_START_BATTLE_ANIMATION,
                            (uptr)StartBattleAnimation, false);
    HookManager::Initialize(HookID::kStartBatlleBackgroundMusic,
                            ADDRESS_BATTLE_START_BACKGROUND_MUSIC,
                            (uptr)StartBattleBackgroundMusicHook);
    HookManager::Initialize(HookID::kPlayBattleAnimation,
                            ADDRESS_BATTLE_PLAY_ANIMATION,
                            (uptr)PlayAnimationHook, false);
    HookManager::Initialize(HookID::kUpdateBattleView,
                            ADDRESS_UPDATE_BATTLE_VIEW,
                            (uptr)UpdateBattleViewHook, false);
    HookManager::Initialize(HookID::kBattleCheckPokemonCaptured,
                            ADDRESS_BATTLE_CHECK_POKEMON_CAPTURED,
                            (uptr)CheckPokemonCaptured,
                            false);
  }

  STATIC_INLINE void PatchOnUpdate() {
    if (!GetInstance().sync_team_hp) return;

    static u32 counter = 20;
    counter--;
    if (counter != 0) return;
    counter = 20;

    bool kill_all = false;
    auto& server_team = battle::Manager::GetTeam(true, 0);
    auto& client_team = battle::Manager::GetTeam(false, 0);
    for (u32 i = 0; i < server_team.count; i++) {
      auto& pkm = *server_team.pokemon[i];
      if (pkm.hp == 0) {
        kill_all = true;
        break;
      }
    }

    if (!kill_all) return;

    for (u32 i = 0; i < server_team.count; i++) {
      server_team.pokemon[i]->hp = 0;
      client_team.pokemon[i]->hp = 0;
    }
  }


  STATIC_INLINE void Patch() {
    HookManager::ForceEnable(HookID::kUpdateBattleView);
    HookManager::ForceEnable(HookID::kUpdateExp);
    HookManager::ForceEnable(HookID::kStartMegaEvolveAnimation);
    HookManager::ForceEnable(HookID::kStartBattleAnimation);
    HookManager::ForceEnable(HookID::kPlayBattleAnimation);
    HookManager::ForceEnable(HookID::kBattleCheckPokemonCaptured);

    auto& feat = GetInstance();
    if (!feat.can_use_item) {
      // Only access to pokeball
      WRITE(vu8, 0x007CB09C, 2); // HP/PP -> Ball
      // WRITE(vu8, 0x007CB0B4, 2) // Ball
      WRITE(vu8, 0x007CB0CC, 2); // Status -> Ball
      WRITE(vu8, 0x007CB0E4, 2); // Battle -> Item
    }

    if (feat.use_pokeball_boost) {
      // Disable master ball feature
      ARM_NOP(0x007227A4);
      ARM_NO_COND(0x007227B8);

      // same ratio for all balls
      WRITE(vu32, 0x007232E4, 0xE3A00A01); // mov r0, #0x1000
      ARM_RET(0x007232E8);
    }

    if (feat.no_shader) {
      ARM_RET(0x003989B0);
      ARM_RET(0x003881EC);
    }

    if (!feat.mega_restriction) {
      ARM_NOP(0x007007C0);
      ARM_NOP(0x006FDA74);
    }

    if (feat.inverse_stats) {
      auto& server_team = battle::Manager::GetTeam(true, 0);
      auto& client_team = battle::Manager::GetTeam(false, 0);
      for (u32 i = 0; i < server_team.count; i++) {
        server_team.pokemon[i]->InverseStats();
        client_team.pokemon[i]->InverseStats();
      }
    }
  }

  static bool CheckPokemonCaptured(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4,
                                   u32 p5) {
#ifdef KAIZO
    // wild battle -> trainer battle (disable capture)
    u8 battle_type = READ(u8, READ(u32, READ(u32, p1 + 4) + 0x10));
    if (kaizo::IsNotFirstEncounter()) {
      WRITE(u8, READ(u32, READ(u32, p1 + 4) + 0x10), 1);
    }
#endif
    bool result = HookManager::Call<bool>(HookID::kBattleCheckPokemonCaptured,
                                          p0, p1, p2, p3, p4, p5);
#ifdef KAIZO
    if (result) {
      kaizo::SetFirstEncounter();
    } else {
      WRITE(u8, READ(u32, READ(u32, p1 + 4) + 0x10), battle_type);
    }
#endif

    return result;
  }

  static void UpdateBattleViewHook(uptr p0) {
    if (GetInstance().fix_pokemon_size) {
      static u32 counter = 20;
      if (counter >= 20) {
        PatchPokemonSize();
        counter = 0;
      }
      counter++;
    }
    HookManager::Call<void>(HookID::kUpdateBattleView, p0);
  }

  static void PatchPokemonSize() {
    for (u32 i = 0; i < 6; i++) {
      u32 pkmMdl = *(u32*)(0x83F84C0 + 4 * i); // Base address of the 3D model
      if (pkmMdl == 0)
        continue;

      u16 pkmNum = *(u16*)(pkmMdl + 0x170); // Pokédex number
      if (pkmNum >= 722)
        continue;

      u32 pkmData = *(u32*)0x617A00 + 0x50 * pkmNum;
      // Personal data of the Pokémon
      float realSize = (float)*(u16*)(pkmData + 0x24);
      // Actual height according to the Pokédex
      float defaultSize = (float)*(u16*)(pkmData + 0x3C);
      // Default displayed size in battle
      float ratio = realSize / defaultSize;

      // Update Pokémon scale
      *(float*)(pkmMdl + 0x34) = ratio;
      *(float*)(pkmMdl + 0x38) = ratio;
      *(float*)(pkmMdl + 0x3C) = ratio;

      // Mark the model to be updated
      *(bool*)(pkmMdl + 0x4C) = true;
    }
  }

  static void PlayAnimationHook(uptr view_manager, u16 id) {
    // disable shiny effect
    if (id == 621 && !GetInstance().show_shiny_animation) return;
    return HookManager::Call<void>(HookID::kPlayBattleAnimation, view_manager,
                                   id);
  }

  static void
  StartBattleBackgroundMusicHook(uptr sound_manager, u32 id, u8 p2) {
    if (GetInstance().sync_overworld_music) {
      id = (1 << 16) + Overworld::GetInstance().background_music;
    }
    return HookManager::Call<void>(HookID::kStartBatlleBackgroundMusic,
                                   sound_manager, id, p2);
  }

  static void StartBattleAnimation(void* p0, void* p1) {
    struct BattleAnimationData {
      u32 view;
      u32 _0;
      u32 state;
      u8 position[5];
      u32 trainer_model[2];

      bool skip_pokeball_animation;
      bool is_long_encounter_animation;
      bool use_trainer_pov;
      bool show_fade_in;
      bool show_shiny_animation;
      bool dont_show_trainer;
    }* data = (BattleAnimationData*)p1;
    auto& config = GetInstance();
    data->skip_pokeball_animation = !config.show_pokeball_animation;
    data->is_long_encounter_animation = config.is_long_encounter_animation;
    data->use_trainer_pov = !config.show_enemy_pov;
    data->show_fade_in = config.show_fade_in;
    data->skip_pokeball_animation = !config.show_pokeball_animation;
    data->show_shiny_animation = config.show_shiny_animation;
    data->dont_show_trainer = !config.show_trainer_animation;

    HookManager::Call<void>(HookID::kStartBattleAnimation, p0, p1);
  }

  static void StartMegaEvolveAnimation(void* view, u8 target,
                                       bool is_long_animation) {
    HookManager::Call<void>(HookID::kStartMegaEvolveAnimation, view, target,
                            GetInstance().is_long_mega_evolve_animation);
  }

  static bool UpdateExpHook(void* self, battle::Team* team,
                            LevelUpData* data) {
#ifdef KAIZO
    kaizo::ApplyLevelCaps(team, data);
#endif
    return HookManager::Call<bool>(HookID::kUpdateExp, self, team, data);
  }

  // static u32 StartTurnHook(uptr server, u32 action) {
  //   // uptr server = READ(u32, 0x082061F8);
  //
  //   // Change Ability
  //   {
  //     struct {
  //       u32 flags;
  //       u16 ability;
  //       u8 id;
  //       u8 _0;
  //       u8 _1;
  //       u32 message[9];
  //     } cmd{};
  //     cmd.id = 0;
  //
  //     cmd.ability = 191;
  //     ((void (*)(uptr, void*))0x07184D8)(server, &cmd);
  //
  //     cmd.ability = 189;
  //     ((void (*)(uptr, void*))0x07184D8)(server, &cmd);
  //   }
  //
  //   // Change HP
  //   {
  //     struct {
  //       u32 flags;
  //       u8 count;
  //       u8 _0[2];
  //       u8 id[10];
  //       s32 hp[10];
  //     } cmd{};
  //
  //     cmd.id[0] = 0;
  //     cmd.count = 1;
  //
  //     cmd.hp[0] = -200;
  //     ((void (*)(uptr, void*))0x007135B0)(server, &cmd);
  //
  //     cmd.hp[0] = 150;
  //     ((void (*)(uptr, void*))0x007135B0)(server, &cmd);
  //   }
  //
  //   // Kill
  //   {
  //     struct {
  //       u32 flags;
  //       u8 id;
  //       u8 _0;
  //       u16 _1;
  //       u32 message[9];
  //     } cmd{};
  //
  //     cmd.id = 0;
  //     ((void (*)(uptr, void*))0x0710D3C)(server, &cmd);
  //   }
  //
  //   // Revive
  //   {
  //     struct {
  //       u32 flags;
  //       u8 id;
  //       u16 hp;
  //       u32 message[9];
  //     } cmd{};
  //
  //     cmd.id = 0;
  //     cmd.hp = 999;
  //
  //     ((void (*)(uptr, void*))0x00711F08)(server, &cmd);
  //   }
  //
  //   // Metamorph
  //   {
  //     struct {
  //       u32 flags : 8;
  //       u32 source : 5;
  //       u32  : 19;
  //       u8 target;
  //       u32 msg[9];
  //     } cmd{};
  //
  //     cmd.source = 0;
  //     cmd.target = 12;
  //
  //     ((void (*)(uptr, void*))0x00711CE4)(server, &cmd);
  //   }
  //
  //   // Attack
  //   {
  //     struct {
  //       u32 action : 4;
  //       u32 target : 4;
  //       u32 move : 16;
  //     } cmd{};
  //
  //     cmd.action = 1; // attack
  //     cmd.target = 1; // enemy
  //     cmd.move = 588;
  //
  //     ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
  //         server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  //
  //     cmd.move = 604;
  //     ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
  //         server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  //
  //     cmd.move = 617;
  //     ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
  //         server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  //   }
  //
  //   return HookManager::GetInstance()
  //          .Get(HookID::kOnStartTurn)
  //          ->CallOriginal<u32>(server, action);
  // }
};
} // namespace feature