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
 * It is the part of the libctrpf file plgldr.h that the plugin needs: the
 * header that Luma3DS writes at the start of the plugin, the events of the
 * loader and the error message.
 */

#pragma once

#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif

/// The header that the plugin loader writes at 0x07000000.
typedef struct {
  u32 magic;
  u32 version;
  u32 heapVA; ///< The start of the memory that the plugin can use as a heap.
  u32 heapSize;
  u32 exeSize;
  u32 isDefaultPlugin;
  s32* plgldrEvent; ///< Used for the synchronization with the loader.
  s32* plgldrReply;
  u8 notifyHomeEvent;
  u8 padding[3];
  u32 config[32];
} PluginHeader;

/// Where the loader puts the header.
#define PLUGIN_HEADER ((volatile PluginHeader*)0x07000000)

/// The events that the loader sends to the plugin.
typedef enum {
  PLG_WAIT = -1,
  PLG_OK = 0,
  PLG_SLEEP_ENTRY = 1,
  PLG_SLEEP_EXIT = 2,
  PLG_ABOUT_TO_SWAP = 3,
  PLG_ABOUT_TO_EXIT = 4,
  PLG_HOME_ENTER = 5,
  PLG_HOME_EXIT = 6,
} PLG_Event;

/// Connects to the plugin loader.
Result plgLdrInit(void);
/// Closes the connection.
void plgLdrExit(void);
/// Shows an error message on the screen (Luma3DS).
Result PLGLDR__DisplayErrMessage(const char* title, const char* body,
                                 u32 error);
/// Returns the event that the loader waits for the plugin to handle.
s32 PLGLDR__FetchEvent(void);
/// Tells the loader that the plugin handled the event.
void PLGLDR__Reply(s32 event);

#ifdef __cplusplus
}
#endif
