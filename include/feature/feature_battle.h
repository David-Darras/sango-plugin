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

#ifndef SANGO_PLUGIN_FEATURE_BATTLE_H
#define SANGO_PLUGIN_FEATURE_BATTLE_H

#include "common.h"
#include "hook_manager.h"

namespace feature {
struct BattleHookContext {
  MAKE_SINGLETON(BattleHookContext)
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
  }

  static void LoadTrainerModelHook(uptr trainer_model,
                                   void* trainer_model_manager) {
    *(u16*)trainer_model = ctx.trainer_model_id;
    *(u16*)(trainer_model + 2) = 0;

    return HookManager::GetInstance()
           .Get(HookID::kOnLoadTrainerModel)
           ->CallOriginal<void>(trainer_model, trainer_model_manager);
  }

  static void PlayAnimationHook(void* graphics, u16 animation) {
    // shiny anim -> rainbow anim
    if (animation == 621) {
      animation = 635;
    }
    HookManager::GetInstance()
        .Get(HookID::kPlayBattleAnimation)
        ->CallOriginal<void>(graphics, animation);
  }

  static u32 StartTurnHook(uptr server, u32 action) {
    // uptr server = READ(u32, 0x082061F8);

    // Change Ability
    {
      struct {
        u32 flags;
        u16 ability;
        u8 id;
        u8 _0;
        u8 _1;
        u32 message[9];
      } cmd{};
      cmd.id = 0;

      cmd.ability = 191;
      ((void (*)(uptr, void*))0x07184D8)(server, &cmd);

      cmd.ability = 189;
      ((void (*)(uptr, void*))0x07184D8)(server, &cmd);
    }

    // Change HP
    {
      struct {
        u32 flags;
        u8 count;
        u8 _0[2];
        u8 id[10];
        s32 hp[10];
      } cmd{};

      cmd.id[0] = 0;
      cmd.count = 1;

      cmd.hp[0] = -200;
      ((void (*)(uptr, void*))0x007135B0)(server, &cmd);

      cmd.hp[0] = 150;
      ((void (*)(uptr, void*))0x007135B0)(server, &cmd);
    }

    // Kill
    {
      struct {
        u32 flags;
        u8 id;
        u8 _0;
        u16 _1;
        u32 message[9];
      } cmd{};

      cmd.id = 0;
      ((void (*)(uptr, void*))0x0710D3C)(server, &cmd);
    }

    // Revive
    {
      struct {
        u32 flags;
        u8 id;
        u16 hp;
        u32 message[9];
      } cmd{};

      cmd.id = 0;
      cmd.hp = 999;

      ((void (*)(uptr, void*))0x00711F08)(server, &cmd);
    }

    // Metamorph
    {
      struct {
        u32 flags : 8;
        u32 source : 5;
        u32  : 19;
        u8 target;
        u32 msg[9];
      } cmd{};

      cmd.source = 0;
      cmd.target = 12;

      ((void (*)(uptr, void*))0x00711CE4)(server, &cmd);
    }

    // Attack
    {
      struct {
        u32 action : 4;
        u32 target : 4;
        u32 move : 16;
      } cmd{};

      cmd.action = 1; // attack
      cmd.target = 1; // enemy
      cmd.move = 588;

      ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
          server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);

      cmd.move = 604;
      ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
          server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);

      cmd.move = 617;
      ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
          server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
    }

    return HookManager::GetInstance()
           .Get(HookID::kOnStartTurn)
           ->CallOriginal<u32>(server, action);
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_BATTLE_H