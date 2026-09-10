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
#include "game/script/amx.h"

namespace game {
class Manager;
}

namespace script {
enum class ScriptKind : u8 {
  kNone = 0,
  kMap = 1,
  kShared = 2,
  kAi = 3,
  kMapInit = 4,
};

enum class ScriptVariable : u16 {
  kParam0 = 0x8000,
  kTemp0 = 0x8008,
  kReturn0 = 0x800C,
  kAnswer = 0x8010,
  kTalkTarget = 0x8011,
  kTalkStartTarget = 0x8012,
  kTalkOptions = 0x8013,
  kTargetBackground = 0x8014,
  kTargetPosition = 0x8015,
  kSmallMessageTail = 0x8016,
};

struct ScriptDescriptor {
  ScriptKind kind; // 0x00
  u8 _0[3];
  s32 script_id; // 0x04
  u32 message_archive_id; // 0x08
  u32 message_file_id; // 0x0C
  u32 native_set_bits; // 0x10
  u16 zone_id; // 0x14
  u16 amx_id; // 0x16
  const c8* amx_file_name; // 0x18
  u16 table_index; // 0x1C
  u16 _1;
};

static_assert(sizeof(ScriptDescriptor) == 0x20, "ScriptDescriptor layout");

struct ScriptRun {
  u8 _0[0x24];
  ScriptDescriptor descriptor; // 0x24
};

struct LoadedMessage {
  void* data;
  void* buffer;
};

struct SharedWorkspaces {
  LoadedMessage* loaded_message;
  LoadedMessage* swapped_message;
  void* yes_no_menu;
  void* list_menu;
  void* talk_message;
  void* nickname_input;
  void* message_window;
  void* event_models;
};

struct EngineWorkspace {
  u8 _0[0x88];
  SharedWorkspaces shared; // 0x88
};

struct PawnVm {
  void* vtable; // 0x00
  AmxRuntime amx; // 0x04
  const void* buffer; // 0x7C
  u32 buffer_size; // 0x80
  void* heap; // 0x84
  s32 suspend_frames; // 0x88
  s16 execute_result; // 0x8C AMX_ERR_NONE once the run is finished
  u16 is_halted; // 0x8E

  INLINE bool IsFinished() const { return execute_result == AMX_ERR_NONE; }
};

static_assert(sizeof(PawnVm) == 0x90, "PawnVm layout");

class Engine;
typedef bool (*WaitFunction)(Engine* engine, u32 label, void* work);

struct ScriptVm : PawnVm {
  ScriptRun* run; // 0x90
  bool is_loaded; // 0x94
  u8 _0[3];
  u32 native_set_bits; // 0x98
  Engine* engine; // 0x9C
  WaitFunction wait_function; // 0xA0
  u32 wait_label; // 0xA4
  void* wait_work; // 0xA8

  INLINE ScriptDescriptor* GetDescriptor() const {
    return run != nullptr ? &run->descriptor : nullptr;
  }

  STATIC_INLINE ScriptVm* FromAmx(AmxRuntime* amx) {
    return (ScriptVm*)amx->user_data_ptrs[0];
  }
};

static_assert(sizeof(ScriptVm) == 0xAC, "ScriptVm layout");

class Engine {
  SINGLETON(Engine)

public:
  STATIC_INLINE Engine* GetInstance() {
    return *(Engine**)ADDRESS_SCRIPT_ENGINE_INSTANCE;
  }

  INLINE game::Manager* GetGameManager() const { return game_manager_; }
  INLINE EngineWorkspace* GetWorkspace() const { return workspace_; }
  INLINE ScriptVm* GetCurrentVm() const { return current_vm_; }
  INLINE u16 GetZoneId() const { return zone_id_; }
  STATIC_INLINE void RaiseSleep(AmxRuntime* amx) {
    ((s32 (*)(AmxRuntime*, s32))ADDRESS_SCRIPT_RAISE_ERROR)(amx, AMX_ERR_SLEEP);
  }

private:
  void* heap_; // 0x00
  void* file_read_heap_; // 0x04
  void* run_heap_; // 0x08
  game::Manager* game_manager_; // 0x0C
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