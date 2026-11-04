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
 * @file plgldr.cc
 * @brief The service of the plugin loader (Luma3DS) that the boot uses.
 *
 * The functions come from the file plgldr.c of libctrpf (zlib license).
 * The declarations are in boot/plgldr.h.
 */

#include "boot/plgldr.h"

#include <cstring>

namespace boot {

namespace {

/// Where the plugin loader writes the header.
constexpr uptr kPluginHeaderAddress = 0x07000000;
/// The error code of the service when the loader is Citra.
constexpr Result kNotImplemented = 0xE0E01BF4;
/// The number of the command that gives the arbiter of the loader.
constexpr u32 kGetArbiterCommand = 9;
/// The number of the command that shows an error.
constexpr u32 kShowErrorCommand = 7;

Handle loader_handle = 0;
Handle loader_arbiter = 0;
s32* loader_event = nullptr;
s32* loader_reply = nullptr;
s32 connection_count = 0;

// Gets the arbiter that the loader uses to wake the plugin.
Result GetArbiter() {
  u32* command = getThreadCommandBuffer();
  command[0] = IPC_MakeHeader(kGetArbiterCommand, 0, 0);
  Result result = svcSendSyncRequest(loader_handle);
  if (R_SUCCEEDED(result)) {
    result = command[1];
    loader_arbiter = command[3];
  }
  return result;
}

// Reads a value that the loader can write at the same time.
s32 ReadExclusive(s32* address) {
  s32 value;
  do {
    value = __ldrex(address);
  } while (__strex(address, value));
  return value;
}

// Writes a value that the loader can read at the same time.
void WriteExclusive(s32* address, s32 value) {
  do {
    __ldrex(address);
  } while (__strex(address, value));
}

} // namespace

const PluginHeader& GetPluginHeader() {
  return *reinterpret_cast<const PluginHeader*>(kPluginHeaderAddress);
}

Result ConnectToLoader() {
  Result result = 0;
  if (AtomicPostIncrement(&connection_count) == 0) {
    result = svcConnectToPort(&loader_handle, "plg:ldr");
  }
  if (R_FAILED(result)) {
    DisconnectFromLoader();
    return result;
  }
  const Result arbiter_result = GetArbiter();
  if (R_SUCCEEDED(arbiter_result) || arbiter_result == kNotImplemented) {
    loader_event = GetPluginHeader().event;
    loader_reply = GetPluginHeader().reply;
  } else {
    DisconnectFromLoader();
  }
  return result;
}

void DisconnectFromLoader() {
  if (AtomicDecrement(&connection_count)) return;
  if (loader_handle != 0) svcCloseHandle(loader_handle);
  if (loader_arbiter != 0) svcCloseHandle(loader_arbiter);
  loader_handle = 0;
  loader_arbiter = 0;
}

Result ShowError(const c8* title, const c8* body, u32 error) {
  u32* command = getThreadCommandBuffer();
  command[0] = IPC_MakeHeader(kShowErrorCommand, 1, 4);
  command[1] = error;
  command[2] = IPC_Desc_Buffer(std::strlen(title), IPC_BUFFER_R);
  command[3] = reinterpret_cast<u32>(title);
  command[4] = IPC_Desc_Buffer(std::strlen(body), IPC_BUFFER_R);
  command[5] = reinterpret_cast<u32>(body);
  Result result = svcSendSyncRequest(loader_handle);
  if (R_SUCCEEDED(result)) result = command[1];
  return result;
}

PluginEvent FetchEvent() {
  return static_cast<PluginEvent>(ReadExclusive(loader_event));
}

void Reply(PluginEvent event) {
  WriteExclusive(loader_reply, static_cast<s32>(PluginEvent::kNone));

  // The events before the swap need no more than the answer.
  if (static_cast<s32>(event) < static_cast<s32>(PluginEvent::kAboutToSwap)) {
    WriteExclusive(loader_event, static_cast<s32>(PluginEvent::kNone));
    return;
  }
  svcArbitrateAddress(loader_arbiter, reinterpret_cast<u32>(loader_reply),
                      ARBITRATION_SIGNAL, 1, 0);
  if (event == PluginEvent::kAboutToSwap) {
    // The plugin waits here until the loader ends the swap.
    WriteExclusive(loader_event, static_cast<s32>(PluginEvent::kWait));
    svcArbitrateAddress(loader_arbiter, reinterpret_cast<u32>(loader_event),
                        ARBITRATION_WAIT_IF_LESS_THAN,
                        static_cast<s32>(PluginEvent::kNone), 0);
  } else if (event == PluginEvent::kAboutToExit) {
    DisconnectFromLoader();
    svcExitThread();
  } else {
    WriteExclusive(loader_event, static_cast<s32>(PluginEvent::kNone));
  }
}

} // namespace boot
