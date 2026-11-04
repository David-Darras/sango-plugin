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
 * @file boot.cc
 * @brief The minimal boot of the plugin. It replaces libctrpf.
 *
 * The plugin loader (Luma3DS) loads the plugin in the memory of the game and
 * calls _start (crt0.s), which calls __entrypoint() below. The plugin must
 * give the control back to the game very soon. So the boot makes a thread, the
 * keep thread, and waits until it has set up the runtime. Then the game runs.
 *
 * What the keep thread does:
 * 1. It sets up the runtime: the thread data, the heap and the static objects
 *    (runtime.cc), and the connection to the plugin loader (plgldr.cc).
 * 2. It lets the game run.
 * 3. It waits for the game to finish its own start (kBootDelay).
 * 4. It starts the main thread, which calls the Initialize() function of the
 *    product. The product installs its hooks and its patches there.
 * 5. It answers the events of the plugin loader (sleep mode, HOME menu, end of
 *    the game) until the game ends.
 *
 * libctrpf does the same, but also starts its own menu, its screen drawing,
 * its sound and its cheat codes. The plugin uses none of them.
 */

#include <3ds.h>

#include "boot/csvc.h"
#include "boot/plgldr.h"
#include "boot/runtime.h"
#include "core/types.h"

/// The function of the product. It installs the hooks and the patches.
extern void Initialize();

extern "C" int __entrypoint(int argument, void* temporary_stack);

namespace boot {

namespace {

/// The size of the stack of the keep thread. The static objects of the plugin
/// are made on this stack, so it must not be smaller.
constexpr u32 kKeepStackSize = 0x1000;
/// The size of the stack of the main thread. It is in the heap.
constexpr u32 kMainStackSize = 0x4000;
/// The priority of the keep thread at the start, then after the start.
constexpr s32 kKeepStartPriority = 0x1A;
constexpr s32 kKeepPriority = 0x2F;
/// The priority of the main thread.
constexpr s32 kMainPriority = 0x30;
/// The time that the game needs to finish its own start, in nanoseconds.
/// Before it, the memory of the game is not ready for the hooks.
constexpr u64 kBootDelay = 5000000000ull;
/// The time between two checks of the plugin loader, in nanoseconds.
constexpr u64 kPollTime = 250000000ull;
/// The result of svcWaitSynchronization when the time ended.
constexpr Result kTimeout = 0x09401BFE;

alignas(8) u8 keep_stack[kKeepStackSize];
Handle continue_event = 0;
Handle keep_thread = 0;

// Calls the function of the product.
void MainThread(void* /*argument*/) { Initialize(); }

// Answers the events of the plugin loader until the game ends. Returns the
// last event, kAboutToExit.
PluginEvent RunEventLoop() {
  Handle memory_changed = 0;
  svcControlProcess(CUR_PROCESS_HANDLE,
                    static_cast<u32>(ProcessOperation::kGetOnMemoryChangeEvent),
                    reinterpret_cast<u32>(&memory_changed), 0);
  for (;;) {
    // The memory of the game changed: nothing to do.
    if (svcWaitSynchronization(memory_changed, kPollTime) != kTimeout) continue;

    const PluginEvent event = FetchEvent();
    switch (event) {
      case PluginEvent::kSleepEntry:
      case PluginEvent::kSleepExit:
      case PluginEvent::kHomeEnter:
      case PluginEvent::kHomeExit:
      case PluginEvent::kAboutToSwap:
        Reply(event);
        break;
      case PluginEvent::kAboutToExit:
        return event;
      default:
        break;
    }
  }
}

// The thread that stays during the whole game.
void KeepThread(void* /*argument*/) {
  InitializeRuntime();
  ConnectToLoader();

  // The game can run now.
  svcSignalEvent(continue_event);

  svcSleepThread(kBootDelay);
  if (threadCreate(MainThread, nullptr, kMainStackSize, kMainPriority, -1,
                   true) == nullptr) {
    ShowError("Fatal plugin error", "The main thread did not start.", 0);
    svcBreak(USERBREAK_PANIC);
  }
  svcSetThreadPriority(keep_thread, kKeepPriority);

  const PluginEvent event = RunEventLoop();

  // The game ends.
  ShutDownRuntime();
  Reply(event); // This call does not return.
  for (;;) {
  }
}

} // namespace

} // namespace boot

// Stops the plugin and shows the address of the code that called abort().
extern "C" void abort(void) {
  void* caller = __builtin_return_address(0);
  boot::ShowError("Fatal plugin error",
                  "abort() called from the following location:",
                  reinterpret_cast<u32>(caller));
  svcSleepThread(500000000ull);
  svcBreak(USERBREAK_PANIC);
  for (;;) {
  }
}

// Called by crt0.s on the thread of the game, before the game runs.
extern "C" int __entrypoint(int /*argument*/, void* /*temporary_stack*/) {
  svcCreateEvent(&boot::continue_event, RESET_ONESHOT);
  u32* stack_top = reinterpret_cast<u32*>(
      &boot::keep_stack[boot::kKeepStackSize]);
  svcCreateThread(&boot::keep_thread, boot::KeepThread, 0, stack_top,
                  boot::kKeepStartPriority, 0);
  // The game continues after the keep thread has set up the runtime.
  svcWaitSynchronization(boot::continue_event, U64_MAX);
  svcCloseHandle(boot::continue_event);
  return 0;
}
