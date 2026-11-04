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
 * @file runtime.h
 * @brief The C runtime of the plugin: the heap, the threads and newlib.
 *
 * The code of the plugin also runs on the threads of the game (the hooks).
 * The game made these threads, so they do not have the thread data that
 * libctru needs for malloc(), snprintf() and errno. The runtime gives them
 * this data the first time they need it.
 */

#pragma once

namespace boot {

/// Sets up the runtime on the thread that starts the plugin: the thread data,
/// the heap and the static objects of the plugin.
void InitializeRuntime();

/// Runs the destructors of the static objects and closes the runtime.
void ShutDownRuntime();

} // namespace boot
