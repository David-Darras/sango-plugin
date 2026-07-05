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

struct AmxInstruction {
  PawnCell opcode : 8;
  PawnCell arguments : 24;
};

//=============================================================================
/**
 * @struct AmxHeader
 * @brief Header structure for AMX (Pawn VM) bytecode files
 *
 * This structure appears at the beginning of every .amx file and defines
 * the memory layout of the loaded script. It serves as a memory map for
 * the Pawn virtual machine during execution.
 *
 * Memory layout after loading:
 * @code
 *   Low address
 *   ┌─────────────────────┐
 *   │   AmxHeader         │  ← This structure
 *   ├─────────────────────┤
 *   │   Bytecode (CODE)   │  ← Executable instructions
 *   ├─────────────────────┤
 *   │   Data (DATA)       │  ← Initialized global variables
 *   ├─────────────────────┤
 *   │   STACK (grows down)│  ← Local variables, call frames
 *   │        ↓            │
 *   │        ↑            │
 *   │   HEAP (grows up)   │  ← Dynamic allocations
 *   ├─────────────────────┤
 *   │   Symbol Tables     │  ← Publics, natives, etc.
 *   └─────────────────────┘
 *   High address
 * @endcode
 */
//=============================================================================

namespace script {
struct AmxHeader {
  u32 total_file_size;
  u16 magic_number; // 0xF1E0
  u8 file_format_version; // 10
  u8 required_virtual_machine_version; // 10

  struct {
    u16 has_debug_info : 1; // 0
    u16 is_code_compacted : 1; // 0
    u16 has_main_function : 1; // 1
    u16 uses_char_arguments : 1; // 1
    u16 has_native_function_registry : 1; // 1
    u16 reserved_flags : 11; //
  };

  u16 symbol_entry_size; // 8 bytes

  // executable bytecode : 1 instruction = 1 pawn cell = 4 bytes
  u32 code_section_offset;
  // initialized global variables
  u32 data_section_offset;
  // dynamic memory allocation area
  u32 heap_start_offset;
  // local variables and call frames
  u32 stack_top_offset;
  // points to first bytecode to execute
  u32 initial_instruction_pointer;
  // Functions that can be called from the host
  u32 public_functions_offset;
  u32 native_functions_offset;
  u32 libraries_offset;
  u32 public_variables_offset;
  u32 tag_names_offset;
  u32 string_table_offset;
  u32 overlay_table_offset;
} __attribute__((packed));

struct AmxPublicFunction {
  u32 name_offset;
  u32 code_address;
};

struct AmxNativeFunction {
  u32 name_offset;
  u32 function_index;
};

struct AmxPublicVariable {
  u32 name_offset;
  u32 data_address;
};

/**
 * @struct AmxRuntime
 * @brief Represents a loaded and running Pawn script instance
 *
 * This structure holds all runtime state for a script: loaded code,
 * memory areas, registers, and callback pointers. One AMX = one script.
 */
struct AmxRuntime {
  //-----------------------------------------------------------------------
  // Memory Sections
  //-----------------------------------------------------------------------

  /// Base pointer to the loaded AMX file in memory (includes header + code)
  uint8_t* memory_base;

  /// Pointer to the CODE section (executable bytecode)
  uint8_t* code_section;

  /// Pointer to the DATA section (global variables + stack + heap)
  uint8_t* data_section;

  //-----------------------------------------------------------------------
  // Callbacks for Host Integration
  //-----------------------------------------------------------------------

  /**
   * Host callback function - Called when script uses "call" instruction
   * Allows script to invoke host functions dynamically
   */
  int (*host_callback)(struct AmxRuntime* amx, PawnCell index,
                       PawnCell* result, const PawnCell* params);

  /**
   * Debug callback - Called when script hits breakpoints or debug events
   * Returns 0 to continue, non-zero to halt
   */
  int (*debug_callback)(struct AmxRuntime* amx);

  /**
   * Overlay loader callback - Loads code overlays on demand
   * Returns 0 on success, non-zero on failure
   */
  int (*overlay_loader)(struct AmxRuntime* amx, int overlay_index);

  //-----------------------------------------------------------------------
  // Virtual Machine Registers
  //-----------------------------------------------------------------------

  /// Program Counter (CIP) - Current instruction address
  PawnCell program_counter;

  /// Frame Pointer (FRM) - Base of current function's stack frame
  PawnCell frame_pointer;

  /// Heap Pointer (HEA) - Current top of heap (grows upward)
  PawnCell heap_pointer;

  /// Heap Low Watermark (HLW) - Bottom of heap (start of heap space)
  PawnCell heap_low_mark;

  /// Stack Pointer (STK) - Current top of stack (grows downward)
  PawnCell stack_pointer;

  /// Stack Top (STP) - Maximum stack address (stack overflow protection)
  PawnCell stack_top;

  //-----------------------------------------------------------------------
  // Status and State
  //-----------------------------------------------------------------------

  /// Current VM status flags (running, sleeping, debugging, etc.)
  int32_t flags;

  /// Last error code (0 = no error)
  int32_t last_error;

  /// Number of parameters for the current native call
  int32_t parameter_count;

  //-----------------------------------------------------------------------
  // Primary Registers (for calculations)
  //-----------------------------------------------------------------------

  /// Primary register - Main arithmetic and result register
  PawnCell primary_register;

  /// Alternate register - Secondary storage for operations
  PawnCell alternate_register;

  //-----------------------------------------------------------------------
  // Saved State for SLEEP Instruction
  //-----------------------------------------------------------------------

  /// Saved stack pointer when script sleeps (for resumption)
  PawnCell saved_stack_pointer;

  /// Saved heap pointer when script sleeps (for resumption)
  PawnCell saved_heap_pointer;

  //-----------------------------------------------------------------------
  // Optimizations
  //-----------------------------------------------------------------------

  /// Relocated address/value for the SYSREQ.D (fast system request)
  PawnCell optimized_sysreq_value;

  //-----------------------------------------------------------------------
  // Overlay Support
  //-----------------------------------------------------------------------

  /// Currently loaded overlay index (-1 = no overlay)
  int32_t current_overlay_index;

  /// Size of current code section (for bounds checking)
  int64_t code_section_size;

  //-----------------------------------------------------------------------
  // User Data (for host applications)
  //-----------------------------------------------------------------------

  /// User-defined data tags (4 slots by default)
  int64_t user_data_tags[4];
  void* user_data_ptrs[4];
} __attribute__((packed));

/**
 * @typedef PawnNativeFunction
 * @brief Signature for native functions that scripts can call
 *
 * @param amx The AMX runtime instance
 * @param params Array of parameter values (index 0 = param count)
 * @return The return value to give back to the script
 */
typedef PawnCell (*PawnNativeFunction)(struct AmxRuntime* amx,
                                       const PawnCell* params);

/**
 * @struct PawnNativeBinding
 * @brief Maps a native function name to its C implementation
 */
struct PawnNativeBinding {
  const char* function_name;
  PawnNativeFunction function;
} __attribute__((packed));
}