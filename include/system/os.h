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

#ifndef SANGO_PLUGIN_OS_H_
#define SANGO_PLUGIN_OS_H_

#include "common.h"
#include "menu/plugin_menu.h"

namespace os {

/**
 * @struct KernelInfo
 * @brief Read-only shared system information (Kernel/OS).
 */
struct KernelInfo {
  struct Version {
    u8 reserved;
    u8 micro;
    u8 minor;
    u8 major;
  };

  /**
   * @brief Configures the menu to display system information.
   */
  static void LoadMenu(menu::PluginMenu& menu) {
    KernelInfo& data = GetInstance();

    menu.Add("Version - Major", data.version.major)
        .Add("Version - Minor", data.version.minor)
        .Add("Version - Micro", data.version.micro)
        .Add("Core Version", data.core_version)
        .Add("Title Id", data.title_id)
        .Add("Is Commercial Console", &data.console_type, 0, 1)
        .Add("Is JTAG Connected", &data.console_type, 1, 1)
        .Add("Application Memory Size", data.memory_size.application)
        .Add("System Memory Size", data.memory_size.system)
        .Add("Base Memory Size", data.memory_size.base);
  }

  /**
   * @brief Returns a reference to the instance at the fixed OS memory address.
   */
  static KernelInfo& GetInstance() {
    return *(KernelInfo*)ADDRESS_OS_READ_ONLY_SHARED_INFO;
  }

  Version version;
  u32 flags;
  u64 title_id;
  u32 core_version;
  u8 console_type;
  u8 jtag;           // 0: prod, 1: dev, 2: debugger
  u8 previous_mode;  // 0: CTR (3DS), 2: TWL (DSi), 4: NTR (DS), 6: AGB (GBA)
  u8 padding1;
  u32 ctrsdk_revision;
  u32 padding2;

  u32 firm_flags;
  u32 unknow0[3];
  u32 application_memory_type;
  u32 unknow1[3];

  struct MemorySize {
    u32 application;
    u32 system;
    u32 base;
  } memory_size;

  u32 unknown2[5];
  Version firm_version;
  u32 firm_core_version;
  u32 firm_ctr_sdk_version;
  u32 padding3;
};

/**
 * @struct ThreadLocalRegion
 * @brief Thread Local Storage (TLS) region for the current thread.
 */
struct ThreadLocalRegion {
  /**
   * @brief Configures the menu to display TLS data and message buffers.
   */
  static void OpenMenu(menu::PluginMenu& menu) {
    ThreadLocalRegion& data = GetInstance();

    menu.Add("TLS[0]", data.tls[0])
        .Add("TLS[1]", data.tls[1])
        .Add("Msg Buffer[0]", data.message_buffer[0])
        .Add("Handler Addr", data.handler_address)
        .Add("Stack Bottom", data.handler_stack_bottom_address)
        .Add("Exception Buffer", data.exception_buffer_address);
  }

  /**
   * @brief Uses MRC p15 (System Coprocessor) to retrieve the TLS address.
   * @return Reference to the current thread's ThreadLocalRegion.
   */
  __attribute__((naked)) static ThreadLocalRegion& GetInstance() {
    __asm__ __volatile__(
        "mrc p15, 0, r0, c13, c0, 3 \n"
        "bx lr                      \n");
  }

  u32 tls[16];
  u32 handler_address;
  u32 handler_stack_bottom_address;
  u32 exception_buffer_address;
  u8 reserved[16];
  u32 eh_globals_addr;
  u32 eh_globals[8];
  u32 message_buffer[64];
  u32 receive_buffer[32];
};

}  // namespace os

#endif  // SANGO_PLUGIN_OS_H_