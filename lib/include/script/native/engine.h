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
 * @file engine.h
 * @brief The script engine of the game.
 */

#pragma once

#include "common.h"
#include "script/constant/script.h"
#include "script/native/amx.h"
#include "script/native/engine_workspace.h"
#include "script/native/script_vm.h"

namespace core {
class GameManager;
}

namespace script {

/// The script engine of the game: it runs the overworld scripts.
class Engine {
  SINGLETON(Engine)

public:
  STATIC_INLINE Engine* GetInstance() {
    return *(Engine**)core::address::kScriptEngineInstance;
  }

  INLINE core::GameManager* GetGameManager() const { return game_manager_; }
  INLINE EngineWorkspace* GetWorkspace() const { return workspace_; }
  /// Returns the virtual machine that runs now.
  INLINE ScriptVm* GetCurrentVm() const { return current_vm_; }
  /// Returns the map of the scripts.
  INLINE u16 GetZoneId() const { return zone_id_; }

  /// Returns true while a script runs.
  INLINE bool IsScriptRunning() const {
    return current_vm_ != nullptr && current_vm_->is_loaded;
  }

  INLINE bool ReserveScript(ScriptId id) {
    if (reserved_script_id_ != (s32)ScriptId::kDoNothing) return false;
    reserved_script_id_ = (s32)id;
    return true;
  }

  /// Stops the Pawn program until the next frame.
  STATIC_INLINE void RaiseSleep(AmxRuntime* amx) {
    ((s32 (*)(AmxRuntime*, s32))core::address::kScriptRaiseError)(amx, AMX_ERR_SLEEP);
  }

private:
  void* heap_; // 0x00
  void* file_read_heap_; // 0x04
  void* run_heap_; // 0x08
  core::GameManager* game_manager_; // 0x0C
  void* variables_; // 0x10
  void* interactive_engine_; // 0x14
  void* ai_engine_; // 0x18
  EngineWorkspace* workspace_; // 0x1C
  void* scene_change_table_; // 0x20
  void* file_buffers_; // 0x24
  void* vm_pool_; // 0x28
  void* character_controller_; // 0x2C
  bool is_created_; // 0x30
  bool is_force_ending_; // 0x31
  bool needs_restore_; // 0x32
  u8 _0;
  ScriptVm* current_vm_; // 0x34
  u16 zone_id_; // 0x38
  u16 _1;
  void* zone_amx_; // 0x3C
  u32 zone_amx_size_; // 0x40
  void* next_event_; // 0x44
  s32 reserved_script_id_; // 0x48
  s32 weak_reserved_script_id_; // 0x4C
};

static_assert(sizeof(Engine) == 0x50, "Engine layout");
} // namespace script
