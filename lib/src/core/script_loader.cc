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

#include "core/patch/script_loader.h"
#include "script/patch/native_script.h"
#include "core/hook_manager.h"
#include "script/native/amx.h"
#include "system/native/file.h"
#include "ui/log_application.h"

namespace core {

void ScriptLoader::Initialize() {
  sys::File::CreateDirectory(u"sdmc:/pkpawn");
  sys::File::CreateDirectory(u"sdmc:/pkpawn/dump");

  HookManager::Initialize(HookId::kLoadScript,
                          address::kScriptPawnBaseLoad,
                          (uptr)LoadHook);
}

void ScriptLoader::PatchLoad() {
  auto& ctx = GetInstance();
  if (ctx.no_key_press) {
    ARM_RETURN_TRUE(address::kScriptWaitKeyPress0);
    ARM_RETURN_TRUE(address::kScriptWaitKeyPress1);
    ARM_RETURN_TRUE(address::kScriptWaitKeyPress2);
    ARM_RETURN_TRUE(address::kScriptWaitKeyPress3);
  }
  if (ctx.no_cutscene) {
    ARM_RETURN_TRUE(address::kScriptPlayCutscene);
  }
}

void ScriptLoader::LoadHook(void* self, const void* buffer, u32 size,
                     const c8* amxname) {
  auto& ctx = GetInstance();
  const u32 id = ComputeId(buffer, size);

  ctx.Dump(id, buffer, size);

  u32 replacement_size = 0;
  if (const void* replacement = ctx.LoadOverride(id, &replacement_size)) {
    buffer = replacement;
    size = replacement_size;
    ctx.ReportInjected(id, replacement_size);
  }

  // Scripts written in C++ take precedence over everything on the SD card.
  script::NativeScript::OnLoad(self, buffer, size);

  HookManager::Call<void>(HookId::kLoadScript, self, buffer, size, amxname);
}

u32 ScriptLoader::ComputeId(const void* data, u32 size) {
  static const u32 kTable[16] = {
      0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC,
      0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
      0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
      0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C,
  };

  const auto* bytes = (const u8*)data;
  u32 crc = 0xFFFFFFFF;
  for (u32 i = 0; i < size; ++i) {
    crc ^= bytes[i];
    crc = (crc >> 4) ^ kTable[crc & 0x0F];
    crc = (crc >> 4) ^ kTable[crc & 0x0F];
  }
  return ~crc;
}

bool ScriptLoader::IsValidHeader(const void* buffer, u32 capacity) {
  if (!buffer) return false;
  if (capacity && capacity < sizeof(script::AmxHeader)) return false;

  const auto* header = (const script::AmxHeader*)buffer;
  if (header->magic_number != AMX_MAGIC) return false;
  if (header->code_section_offset < sizeof(script::AmxHeader)) return false;
  if (header->code_section_offset > header->data_section_offset) return false;
  if (header->data_section_offset > header->heap_start_offset) return false;
  if (header->heap_start_offset > header->stack_top_offset) return false;
  if (header->total_file_size < sizeof(script::AmxHeader)) return false;
  if (header->total_file_size > header->stack_top_offset) return false;
  if (capacity && header->total_file_size > capacity) return false;
  return true;
}

void ScriptLoader::Dump(u32 id, const void* buffer, u32 size) {
  if (!dump_scripts || !size) return;
  if (!MarkSeen(id)) return;

  c16 filename[sys::address::kBufferSize];
  Utils::Format(filename, u"sdmc:/pkpawn/dump/%08x.amx", id);
  {
    sys::File file(filename, true);
    file.Write(buffer, size);
  }
  AppendIndexEntry(id, size);
  dumped_count++;

  if (log_activity) {
    ui::LogApplication::Print(u"dumped script %08x (%d bytes)", id, size);
  }
}

bool ScriptLoader::MarkSeen(u32 id) {
  for (u32 i = 0; i < kDumpHistorySize; ++i) {
    if (dump_history_[i] == id) return false;
  }
  dump_history_[dump_history_next_] = id;
  dump_history_next_ = (dump_history_next_ + 1) % kDumpHistorySize;
  return true;
}

void ScriptLoader::AppendIndexEntry(u32 id, u32 size) {
  c16 line[sys::address::kBufferSize];
  c8 ascii[sys::address::kBufferSize];
  Utils::Format(line, u"%08x %u\n", id, size);
  u32 length = 0;
  while (length < sys::address::kBufferSize - 1 && line[length]) {
    ascii[length] = (c8)line[length];
    length++;
  }

  sys::File file;
  file.Open(u"sdmc:/pkpawn/dump/index.txt");
  if (!file.IsOpen()) return;
  file.Write(ascii, length, index_offset_);
  index_offset_ += length;
}

const void* ScriptLoader::LoadOverride(u32 id, u32* out_size) {
  if (!inject_scripts) return nullptr;

  c16 filename[sys::address::kBufferSize];
  Utils::Format(filename, u"sdmc:/pkpawn/%08x.amx", id);

  u8* buffer = OverrideBuffer();
  const u32 size = sys::File::ReadAll(filename, buffer, kMaxScriptBytes);
  if (!size) return nullptr;

  if (!IsValidHeader(buffer, size)) {
    if (log_activity) {
      ui::LogApplication::Print(u"script %08x: override is not a valid .amx",
                                id);
    }
    return nullptr;
  }

  const auto* header = (const script::AmxHeader*)buffer;
  if (header->total_file_size != size) {
    if (log_activity) {
      ui::LogApplication::Print(
          u"script %08x: override is %d bytes, header says %d", id, size,
          header->total_file_size);
    }
    return nullptr;
  }

  *out_size = size;
  return buffer;
}

void ScriptLoader::ReportInjected(u32 id, u32 size) {
  injected_count++;
  if (log_activity) {
    ui::LogApplication::Print(u"script %08x replaced (%d bytes)", id, size);
  }
}

u8* ScriptLoader::OverrideBuffer() {
  alignas(4) static u8 buffer[kMaxScriptBytes];
  return buffer;
}

} // namespace core
