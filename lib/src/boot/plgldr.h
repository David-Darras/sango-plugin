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
 * @file plgldr.h
 * @brief The service of the plugin loader (Luma3DS) that the boot uses.
 *
 * The loader writes a header at the start of the plugin and sends events to
 * it: the sleep mode, the HOME menu and the end of the game. The plugin must
 * answer each event. This file has the part of libctrpf that does this.
 */

#pragma once

#include <3ds.h>

#include "core/types.h"

namespace boot {

/// The header that the plugin loader writes at the start of the plugin.
struct PluginHeader {
  u32 magic;
  u32 version;
  u32 heap_address; ///< The start of the memory that the plugin uses as a heap.
  u32 heap_size;
  u32 exe_size;
  u32 is_default_plugin;
  s32* event; ///< The event that the loader sends. Used to synchronize.
  s32* reply; ///< The answer of the plugin. Used to synchronize.
  u8 notify_home_event;
  u8 padding[3];
  u32 config[32];
};

/// The events that the plugin loader sends to the plugin.
enum class PluginEvent : s32 {
  kWait = -1,
  kNone = 0,
  kSleepEntry = 1,
  kSleepExit = 2,
  kAboutToSwap = 3,
  kAboutToExit = 4,
  kHomeEnter = 5,
  kHomeExit = 6,
};

/// Returns the header that the plugin loader wrote at the start of the plugin.
const PluginHeader& GetPluginHeader();

/// Connects to the plugin loader. Returns the result of the connection.
Result ConnectToLoader();

/// Closes the connection to the plugin loader.
void DisconnectFromLoader();

/// Shows an error message on the screen with the code `error`.
Result ShowError(const c8* title, const c8* body, u32 error);

/// Returns the event that the plugin loader waits for the plugin to handle.
PluginEvent FetchEvent();

/// Tells the plugin loader that the plugin handled `event`. For
/// PluginEvent::kAboutToExit, the function ends the thread.
void Reply(PluginEvent event);

} // namespace boot
