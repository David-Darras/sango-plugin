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
 * @file singleton.h
 * @brief The macros that make a class a singleton.
 *
 * A singleton is a class with one object only.
 *
 * - MAKE_SINGLETON(Name): for a class of the plugin. The macro adds
 *   GetInstance(), which returns the only object.
 * - SINGLETON(Name): for a game structure. The structure must write its own
 *   GetInstance(), which returns the object of the game.
 *
 * @code
 * class MyFeature {
 *   MAKE_SINGLETON(MyFeature)
 * public:
 *   bool is_enabled = false;
 * };
 *
 * MyFeature::GetInstance().is_enabled = true;
 * @endcode
 */

#pragma once

/// Removes the copy and the move of a class, and makes its constructor
/// private. The class must supply GetInstance().
#define SINGLETON(ClassName)\
public:\
ClassName(const ClassName&)            = delete;\
ClassName& operator=(const ClassName&) = delete;\
ClassName(ClassName&&)                 = delete;\
ClassName& operator=(ClassName&&)      = delete;\
private:\
ClassName() = default; \
public:

/// Makes a class a singleton, with a GetInstance() function that returns
/// the only object.
#define MAKE_SINGLETON(ClassName)\
public:\
ClassName(const ClassName&)            = delete;\
ClassName& operator=(const ClassName&) = delete;\
ClassName(ClassName&&)                 = delete;\
ClassName& operator=(ClassName&&)      = delete;\
static inline __attribute__((always_inline)) ClassName& GetInstance() {\
static ClassName instance;\
return instance;\
}\
private:\
ClassName() = default;\
public:
