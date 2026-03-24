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

#include "menu/plugin_menu.h"

namespace {

bool test_bool = false;
u8 test_u8 = 1;
u16 test_u16 = 2;
u32 test_u32 = 3;
u64 test_u64 = 4;

s8 test_s8 = -1;
s16 test_s16 = -2;
s32 test_s32 = -3;
s64 test_s64 = -4;

f32 test_f32 = 1.5f;
f64 test_f64 = 2.5;

u32 test_bits = 0b10101010;

c16 test_unicode[32] = u"Hello";

void *test_ptr = &test_u32;

const c8 *on_off[] = {"Off", "On"};

}  // namespace

static void TestToggle(void *) { test_bool ^= true; }

void TestMenu(menu::PluginMenu &menu, void* args) {
  menu.Add("Refresh test", test_u8)
      .WithCallback(TestToggle)
      .WithRefresh()
      .Add("Bool", test_bool)
      .Add("U8", test_u8)
      .WithBounds(0, 255)
      .Add("U16", test_u16)
      .Add("U32", test_u32)
      .Add("U64", test_u64)
      .Add("S8", test_s8)
      .Add("S16", test_s16)
      .Add("S32", test_s32)
      .Add("S64", test_s64)
      .Add("F32", test_f32)
      .Add("F64", test_f64)
      .Add("Bits (0-3)", &test_bits, 0, 4)
      .Add("Unicode", test_unicode, 32)
      .Add("Pointer", test_ptr)
      .Add("Bool w/ array", test_bool)
      .WithArray(on_off, 2);
}