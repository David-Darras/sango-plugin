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

typedef u32 PawnCell;
#define AMX_MAGIC 0xF1E0

#define AMX_FILE_VERSION 10
#define AMX_SYMBOL_ENTRY_SIZE 8

#define AMX_FLAG_OVERLAY 0x01
#define AMX_FLAG_DEBUG 0x02
#define AMX_FLAG_COMPACT 0x04
#define AMX_FLAG_SLEEP 0x08
#define AMX_FLAG_NOCHECKS 0x10

#define AMX_ERR_NONE 0
#define AMX_ERR_SLEEP 12

enum AmxOpcode : u32 {
  kAmxOpPushC = 39,
  kAmxOpStack = 44,
  kAmxOpProc = 46,
  kAmxOpRetn = 48,
  kAmxOpJnz = 54,
  kAmxOpZeroPri = 89,
  kAmxOpHalt = 120,
  kAmxOpSysreqC = 123,
};

struct AmxInstruction {
  PawnCell opcode : 8;
  PawnCell arguments : 24;
};

namespace script {
struct AmxHeader {
  u32 total_file_size;
  u16 magic_number; // 0xF1E0
  u8 file_format_version; // 10
  u8 required_virtual_machine_version; // 10

  u16 flags;
  u16 symbol_entry_size;

  u32 code_section_offset;
  u32 data_section_offset;
  u32 heap_start_offset;
  u32 stack_top_offset;
  u32 initial_instruction_pointer;
  u32 public_functions_offset;
  u32 native_functions_offset;
  u32 libraries_offset;
  u32 public_variables_offset;
  u32 tag_names_offset;
  u32 string_table_offset;
  u32 overlay_table_offset;
} __attribute__((packed));

static_assert(sizeof(AmxHeader) == 60, "AmxHeader must match AMX_HEADER");

struct AmxSymbol {
  u32 address;
  u32 name_hash;
};

constexpr u32 CalcNameHash(const c8* name, u32 value = 0) {
  return *name == '\0'
           ? value
           : CalcNameHash(name + 1, (value * 0x83) ^ static_cast<u8>(*name));
}

struct AmxRuntime {
  uint8_t* memory_base;
  uint8_t* code_section;
  uint8_t* data_section;
  int (*host_callback)(struct AmxRuntime* amx, PawnCell index,
                       PawnCell* result, const PawnCell* params);
  int (*debug_callback)(struct AmxRuntime* amx);
  int (*overlay_loader)(struct AmxRuntime* amx, int overlay_index);
  PawnCell program_counter;
  PawnCell frame_pointer;
  PawnCell heap_pointer;
  PawnCell heap_low_mark;
  PawnCell stack_pointer;
  PawnCell stack_top;
  int32_t flags;
  int32_t user_data_tags[4];
  void* user_data_ptrs[4];
  int32_t last_error;
  int32_t parameter_count;
  PawnCell primary_register;
  PawnCell alternate_register;
  PawnCell saved_stack_pointer;
  PawnCell saved_heap_pointer;
  PawnCell optimized_sysreq_value;
  int32_t current_overlay_index;
  int32_t code_section_size;
};

static_assert(sizeof(AmxRuntime) == 0x78, "AmxRuntime must match AMX");

typedef PawnCell (*PawnNativeFunction)(struct AmxRuntime* amx,
                                       const PawnCell* params);

struct PawnNativeBinding {
  const char* function_name;
  PawnNativeFunction function;
} __attribute__((packed));
}