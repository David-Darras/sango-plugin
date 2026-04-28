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

#ifndef SANGO_PLUGIN_COMMON_H
#define SANGO_PLUGIN_COMMON_H

#include <cstdint>

namespace menu {
class PluginMenu;
}

typedef void (*menu_callback_t)(menu::PluginMenu& menu, void* args);
typedef void (*callback_t)(void* args);

#define TYPEDEF_INTEGER(n, t)        \
  typedef unsigned t u##n;           \
  typedef volatile unsigned t vu##n; \
  typedef signed t s##n;             \
  typedef volatile signed t vs##n;

TYPEDEF_INTEGER(8, char)
TYPEDEF_INTEGER(16, short)
TYPEDEF_INTEGER(32, int)
TYPEDEF_INTEGER(64, long long)

#define TYPEDEF_FLOAT(n, t) \
  typedef t f##n;           \
  typedef volatile t vf##n;

TYPEDEF_FLOAT(32, float)
TYPEDEF_FLOAT(64, double)

typedef char16_t c16;
typedef char c8;

typedef uintptr_t uptr;

#define WRITE(type, address, value) *(type*)(address) = (value)
#define READ(type, address) *(type*)(address)

#define ARM_NOP(address) *(vu32*)(address) = 0xE1A00000
#define ARM_RET(address) *(vu32*)(address) = 0xE12FFF1E  // bx lr
#define ARM_NO_COND(address) \
  *(vu32*)(address) = (*(vu32*)(address) & 0x0FFFFFFF) | 0xE0000000

#define SET_BITS(b, p, n, v) \
  ((b) = ((b) & ~(((1u << (n)) - 1) << (p))) | ((v) << (p)))
#define GET_BITS(b, p, n) (((b) >> (p)) & ((1u << (n)) - 1))

#define SIZE(x) ((sizeof(x)) / (sizeof(x[0])))

extern "C" s32 svcFlushProcessDataCache(u32 handle, u32 addr, u32 size);

extern "C" s32 svcInvalidateEntireInstructionCache();

struct Vec2 {
  f32 x, y;
  Vec2() : x(0), y(0) {}
  Vec2(f32 x0, f32 y0) : x(x0), y(y0) {}
};

struct Vec3 {
  f32 x, y, z;

  Vec3() : x(0), y(0), z(0) {}
  Vec3(f32 x0, f32 y0, f32 z0) : x(x0), y(y0), z(z0) {}

  static float Dot(const Vec3& a, const Vec3& b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
  }
};

struct Vec4 {
  f32 x, y, z, w;
  Vec4() : x(0), y(0), z(0), w(0) {}
  Vec4(f32 x0, f32 y0, f32 z0, f32 w0) : x(x0), y(y0), z(z0), w(w0) {}
};

struct Color {
  f32 r, g, b, a;
  Color() : r(1), g(1), b(1), a(1) {}
  Color(f32 r0, f32 g0, f32 b0, f32 a0) : r(r0), g(g0), b(b0), a(a0) {}
};

struct Color8 {
  u8 r, g, b, a;
  Color8() : r(255), g(255), b(255), a(255) {}
  Color8(u8 r0, u8 g0, u8 b0, u8 a0) : r(r0), g(g0), b(b0), a(a0) {}
};

struct Mtx33 {
  f32 m[3][3];
};

struct Mtx34 {
  f32 m[3][4];
};

struct Mtx44 {
  f32 m[4][4];
};

struct Aabb {
  Vec4 min;
  Vec4 max;
};

#define ADDRESS_STRING_VTABLE (0x5DE3BC)

struct String {
  static String s_tmp;
  static c16 s_buffer[128];

  static String* GetTmpStr() { return &s_tmp; }
  static c16* GetTmpBuf() { return s_buffer; }

  String() {
    vtable = (void*)ADDRESS_STRING_VTABLE;
    buffer = s_buffer;
    capacity = 128;
    size = 0;
    is_initialized = true;
  }

  void* vtable;
  c16* buffer;
  u16 capacity;
  u16 size;
  bool is_initialized;
};

#endif  // SANGO_PLUGIN_COMMON_H
