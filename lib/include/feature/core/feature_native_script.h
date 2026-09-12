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
#include "feature/core/hook_manager.h"
#include "game/constant/script.h"
#include "game/script/amx.h"
#include "game/script/native.h"
#include "game/script/engine.h"
#include "script/context.h"
#include "system/coroutine.h"
#include "ui/log_application.h"

namespace feature {
/**
 *   halt 0            ; return address 0 lands here (Pawn convention)
 *   main: proc
 *   loop: push.c 0    ; no arguments
 *         sysreq.c 0  ; native #0 -> NativeEntry
 *         stack 4
 *         jnz loop    ; keep calling while the C++ side says so
 *         zero.pri
 *         retn
 */
class NativeScript {
  MAKE_SINGLETON(NativeScript)

public:
  static constexpr u32 kMaxScripts = 32;
  static constexpr u32 kStackSize = 32 * 1024;
  static constexpr u32 kMessageCapacity = 512;
  static constexpr s32 kBorrowedScriptId = (s32)ScriptId::kDoNothing;

  bool log_activity = false;
  u32 run_count = 0;

  STATIC_INLINE void Initialize() {
    GetInstance().BuildStub();
    HookManager::Initialize(HookID::kScriptDescriptorSetup,
                            ADDRESS_SCRIPT_DESCRIPTOR_SETUP,
                            (uptr)DescriptorSetupHook);
  }

  static bool Register(ScriptId id, script::Function function) {
    auto& ctx = GetInstance();
    if (function == nullptr || ctx.Find(id) != nullptr) return false;
    if (ctx.count_ >= kMaxScripts) return false;
    ctx.entries_[ctx.count_++] = Entry{id, function};
    return true;
  }

  static bool IsRegistered(ScriptId id) {
    return GetInstance().Find(id) != nullptr;
  }

  static u32 GetCount() { return GetInstance().count_; }

  static bool OnLoad(void* self, const void*& buffer, u32& size) {
    auto& ctx = GetInstance();
    auto* vm = (script::ScriptVm*)self;
    script::Engine* engine = script::Engine::GetInstance();
    if (engine == nullptr || engine->GetCurrentVm() != vm) return false;

    script::ScriptDescriptor* descriptor = vm->GetDescriptor();
    if (descriptor == nullptr) return false;
    const Entry* entry = ctx.Find((ScriptId)descriptor->script_id);
    if (entry == nullptr) return false;

    if (!ctx.natives_resolved_) {
      if (!ctx.natives_.Resolve()) {
        ui::LogApplication::Print(u"script %d: some natives are missing",
                                  descriptor->script_id);
      }
      ctx.natives_resolved_ = true;
    }

    if (ctx.IsSuspendedElsewhere(engine, vm)) {
      ui::LogApplication::Print(u"script %d: nested C++ scripts unsupported",
                                descriptor->script_id);
      return false;
    }

    ctx.current_ = entry;
    ctx.engine_ = engine;
    ctx.vm_ = vm;
    ctx.context_.Bind(&vm->amx);
    ctx.coroutine_.Start(RunScript, &ctx);
    ctx.run_count++;
    if (ctx.log_activity) {
      ui::LogApplication::Print(u"script %d -> C++", descriptor->script_id);
    }

    buffer = ctx.stub_;
    size = ((const script::AmxHeader*)ctx.stub_)->total_file_size;
    return true;
  }

private:
  struct Entry {
    ScriptId id;
    script::Function function;
  };

  static constexpr u32 kStubSize = 0x80;
  static constexpr u32 kStubStackSize = 0x1000; // like the game's scripts

  const Entry* Find(ScriptId id) const {
    for (u32 i = 0; i < count_; i++) {
      if (entries_[i].id == id) return &entries_[i];
    }
    return nullptr;
  }

  bool IsSuspendedElsewhere(const script::Engine* engine,
                            const script::ScriptVm* vm) const {
    if (current_ == nullptr || coroutine_.IsFinished()) return false;
    if (engine_ != engine || vm_ == vm) return false;
    return vm_->is_loaded;
  }

  static void DescriptorSetupHook(script::ScriptDescriptor* descriptor,
                                  u16 zone_id, s32 script_id,
                                  void* zone_data) {
    const bool custom = IsRegistered((ScriptId)script_id);
    HookManager::Call<void>(HookID::kScriptDescriptorSetup, descriptor,
                            zone_id, custom ? kBorrowedScriptId : script_id,
                            zone_data);
    if (custom) descriptor->script_id = script_id;
  }

  static PawnCell NativeEntry(script::AmxRuntime* amx, const PawnCell*) {
    auto& ctx = GetInstance();
    if (ctx.current_ == nullptr) return 0;
    if (script::ScriptVm::FromAmx(amx) != ctx.vm_) return 0;

    ctx.context_.Bind(amx);
    const bool alive = ctx.coroutine_.Resume();

    if (!ctx.coroutine_.IsStackHealthy()) {
      ui::LogApplication::Print(u"script %d: stack overflow!",
                                (s32)ctx.current_->id);
    }
    if (alive) return 1;

    if (ctx.log_activity) {
      ui::LogApplication::Print(u"script %d done (stack %u bytes)",
                                (s32)ctx.current_->id,
                                ctx.coroutine_.GetStackHighWater());
    }
    ctx.current_ = nullptr;
    ctx.vm_ = nullptr;
    return 0;
  }

  static void RunScript(void* arg) {
    auto* ctx = (NativeScript*)arg;
    ctx->current_->function(ctx->context_);
  }

  void BuildStub() {
    auto* header = (script::AmxHeader*)stub_;
    auto* native = (script::AmxSymbol*)(stub_ + sizeof(script::AmxHeader));
    u32 offset = sizeof(script::AmxHeader) + sizeof(script::AmxSymbol);
    auto* name_table = (u16*)(stub_ + offset);
    offset += 2 * sizeof(u16);
    const u32 code_offset = offset;
    auto* code = (u32*)(stub_ + code_offset);
    u32 n = 0;

    code[n++] = kAmxOpHalt;
    code[n++] = 0;
    const u32 main_offset = n * sizeof(u32);
    code[n++] = kAmxOpProc;
    const u32 loop_offset = n * sizeof(u32);
    code[n++] = kAmxOpPushC;
    code[n++] = 0;
    code[n++] = kAmxOpSysreqC;
    code[n++] = 0;
    code[n++] = kAmxOpStack;
    code[n++] = sizeof(u32);
    const u32 jnz_offset = n * sizeof(u32);
    code[n++] = kAmxOpJnz;
    code[n++] = (u32)((s32)loop_offset - (s32)jnz_offset);
    code[n++] = kAmxOpZeroPri;
    code[n++] = kAmxOpRetn;
    const u32 code_size = n * sizeof(u32);

    native->address = (u32)&NativeEntry;
    native->name_hash = script::CalcNameHash("CppScript");
    name_table[0] = 63;
    name_table[1] = 0;

    header->total_file_size = code_offset + code_size;
    header->magic_number = AMX_MAGIC;
    header->file_format_version = AMX_FILE_VERSION;
    header->required_virtual_machine_version = AMX_FILE_VERSION;
    header->flags = AMX_FLAG_SLEEP | AMX_FLAG_NOCHECKS;
    header->symbol_entry_size = AMX_SYMBOL_ENTRY_SIZE;
    header->code_section_offset = code_offset;
    header->data_section_offset = code_offset + code_size;
    header->heap_start_offset = header->data_section_offset;
    header->stack_top_offset = header->heap_start_offset + kStubStackSize;
    header->initial_instruction_pointer = main_offset;
    header->public_functions_offset = sizeof(script::AmxHeader);
    header->native_functions_offset = sizeof(script::AmxHeader);
    header->libraries_offset = sizeof(script::AmxHeader) +
                               sizeof(script::AmxSymbol);
    header->public_variables_offset = header->libraries_offset;
    header->tag_names_offset = header->libraries_offset;
    header->string_table_offset = header->libraries_offset;
    header->overlay_table_offset = header->libraries_offset;
  }

  Entry entries_[kMaxScripts];
  u32 count_ = 0;
  const Entry* current_ = nullptr;
  script::Engine* engine_ = nullptr;
  script::ScriptVm* vm_ = nullptr;
  script::Natives natives_ = {};
  bool natives_resolved_ = false;
  alignas(8) u8 stack_[kStackSize];
  alignas(4) u8 stub_[kStubSize];
  alignas(4) c16 message_buffer_[kMessageCapacity];
  Coroutine coroutine_{stack_, kStackSize};
  script::Context context_{natives_, coroutine_, message_buffer_,
                           kMessageCapacity};
};
} // namespace feature