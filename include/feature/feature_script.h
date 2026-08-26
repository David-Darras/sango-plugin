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
#include "hook_manager.h"
#include "system/file.h"
#include "ui/log_application.h"
#include "game/script/amx.h"
#include "game/overworld/map_manager.h"

namespace feature {
class Script {
  MAKE_SINGLETON(Script)

public:
  static constexpr u32 kMaxScriptBytes = 64 * 1024;
  static constexpr u32 kDumpHistorySize = 128;
  bool dump_scripts = true;
  bool inject_scripts = true;
  bool log_activity = true;
  u32 dumped_count = 0;
  u32 injected_count = 0;

  STATIC_INLINE void Initialize() {
    //File::CreateDirectory(u"sdmc:/pkpawn");
    //File::CreateDirectory(u"sdmc:/pkpawn/dump");

    // HookManager::Initialize(HookID::kLoadScript,
    //                         ADDRESS_SCRIPT_PAWN_BASE_LOAD,
    //                         (uptr)LoadHook);
  }

  static void LoadHook(void* self, const void* buffer, u32 size,
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

    HookManager::Call<void>(HookID::kLoadScript, self, buffer, size, amxname);
  }

private:

  static u32 ComputeId(const void* data, u32 size) {
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

  static bool IsValidHeader(const void* buffer, u32 capacity) {
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

  void Dump(u32 id, const void* buffer, u32 size) {
    if (!dump_scripts || !size) return;
    if (!MarkSeen(id)) return;

    c16 filename[BUFFER_SIZE];
    Utils::Format(filename, u"sdmc:/pkpawn/dump/%08x.amx", id);
    {
      File file(filename, true);
      file.Write(buffer, size);
    }
    AppendIndexEntry(id, size);
    dumped_count++;

    if (log_activity) {
      ui::LogApplication::Print(u"dumped script %08x (%d bytes)", id, size);
    }
  }

  bool MarkSeen(u32 id) {
    for (u32 i = 0; i < kDumpHistorySize; ++i) {
      if (dump_history_[i] == id) return false;
    }
    dump_history_[dump_history_next_] = id;
    dump_history_next_ = (dump_history_next_ + 1) % kDumpHistorySize;
    return true;
  }

  void AppendIndexEntry(u32 id, u32 size) {
    c16 line[BUFFER_SIZE];
    c8 ascii[BUFFER_SIZE];
    u32 length = 0;
    while (length < BUFFER_SIZE - 1 && line[length]) {
      ascii[length] = (c8)line[length];
      length++;
    }

    File file;
    file.Open(u"sdmc:/pkpawn/dump/index.txt");
    if (!file.IsOpen()) return;
    file.Write(ascii, length, index_offset_);
    index_offset_ += length;
  }

  const void* LoadOverride(u32 id, u32* out_size) {
    if (!inject_scripts) return nullptr;

    c16 filename[BUFFER_SIZE];
    Utils::Format(filename, u"sdmc:/pkpawn/%08x.amx", id);

    u8* buffer = OverrideBuffer();
    const u32 size = File::ReadAll(filename, buffer, kMaxScriptBytes);
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

  void ReportInjected(u32 id, u32 size) {
    injected_count++;
    if (log_activity) {
      ui::LogApplication::Print(u"script %08x replaced (%d bytes)", id, size);
    }
  }

  STATIC_INLINE u8* OverrideBuffer() {
    alignas(4) static u8 buffer[kMaxScriptBytes];
    return buffer;
  }

  u32 dump_history_[kDumpHistorySize] = {};
  u32 dump_history_next_ = 0;
  s64 index_offset_ = 0;
};
} // namespace feature
