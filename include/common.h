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

#ifdef __CLION_IDE__
#define USE_SANGO_PLUGIN
// #define USE_DEFAULT_CTRPF
#define KAIZO
#endif

#include <types.h>
#include <functional>
#include "address.h"
#include <CTRPluginFramework/System/Process.hpp>

#define TYPEDEF_FLOAT(n, t) \
typedef t f##n;           \
typedef volatile t vf##n;

TYPEDEF_FLOAT(32, float)
TYPEDEF_FLOAT(64, double)

typedef char16_t c16;
typedef char c8;

typedef uintptr_t uptr;

#define INLINE inline __attribute__((always_inline))
#define STATIC_INLINE static inline __attribute__((always_inline))
#define WEAK __attribute__((weak))

// #define INLINE inline __attribute__((always_inline))
// #define STATIC_INLINE STATIC_INLINE __attribute__((always_inline))

#define SINGLETON(ClassName)\
public:\
ClassName(const ClassName&)            = delete;\
ClassName& operator=(const ClassName&) = delete;\
ClassName(ClassName&&)                 = delete;\
ClassName& operator=(ClassName&&)      = delete;\
private:\
ClassName() = default; \
public:

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

class MemoryManager {
  MAKE_SINGLETON(MemoryManager);
  static bool ToggleProtection(u32 address, bool on);
  static bool Unprotect(u32 address, u32 size);
  static bool Protect(u32 address, u32 size);
};

class MemoryRange {
public:
  MemoryRange(u32 address, u32 size) : address_(address), size_(size) {
    MemoryManager::Unprotect(address, size);
  }

  ~MemoryRange() {
    MemoryManager::Protect(address_, size_);
  }

  MemoryRange(const MemoryRange&) = delete;
  MemoryRange& operator=(const MemoryRange&) = delete;

private:
  u32 address_;
  u32 size_;
};

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define MEMORY_SCOPE(addr, size) MemoryRange CONCAT(mem_scope_, __COUNTER__)(addr, size)

namespace ui {
class MainApplication;
}

typedef void (*menu_callback_t)(ui::MainApplication& app, void* args);
// typedef void (*callback_t)(void* args);
typedef std::function<void(void*)> callback_t;
typedef std::function<void()> cheat_code_callback_t;
#define UNPROTECT(address) MemoryManager::ToggleProtection((address), true)
#define PROTECT(address)  MemoryManager::ToggleProtection((address), false)

#define WRITE64(address, value) *(vu64*)(address) = (value)
#define WRITE32(address, value) *(vu32*)(address) = (value)
#define WRITE16(address, value) *(vu16*)(address) = (value)
#define WRITE8(address, value) *(vu8*)(address) = (value)
#define WRITEF(address, value) *(vf32*)(address) = (value)
#define WRITEB(address, value) *(volatile bool*)(address) = (value)

#define READ64(address) *(vu64*)(address)
#define READ32(address) *(vu32*)(address)
#define READ16(address) *(vu16*)(address)
#define READ8(address) *(vu8*)(address)
#define READF(address) *(vf32*)(address)
#define READB(address) *(volatile bool*)(address)

#define SAFE_WRITE64(address, value) do {\
  MemoryManager::ToggleProtection((address), true);\
  *(vu64*)(address) = (value);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_WRITE32(address, value) do {\
  MemoryManager::ToggleProtection((address), true);\
  *(vu32*)(address) = (value);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_WRITE16(address, value) do {\
  MemoryManager::ToggleProtection((address), true);\
  *(vu16*)(address) = (value);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_WRITE8(address, value) do {\
  MemoryManager::ToggleProtection((address), true);\
  *(vu8*)(address) = (value);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_WRITEF(address, value) do {\
  MemoryManager::ToggleProtection((address), true);\
  *(vf32*)(address) = (value);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_WRITEB(address, value) do {\
  MemoryManager::ToggleProtection((address), true);\
  *(volatile bool*)(address) = (value);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_READ64(address, result) do {\
  MemoryManager::ToggleProtection((address), true);\
  (result) = *(vu64*)(address);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_READ32(address, result) do {\
  MemoryManager::ToggleProtection((address), true);\
  (result) = *(vu32*)(address);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_READ16(address, result) do {\
  MemoryManager::ToggleProtection((address), true);\
  (result) = *(vu16*)(address);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_READ8(address, result) do {\
  MemoryManager::ToggleProtection((address), true);\
  (result) = *(vu8*)(address);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_READF(address, result) do {\
  MemoryManager::ToggleProtection((address), true);\
  (result) = *(vf32*)(address);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define SAFE_READB(address, result) do {\
  MemoryManager::ToggleProtection((address), true);\
  (result) = *(volatile bool*)(address);\
  MemoryManager::ToggleProtection((address), false);\
} while(0)

#define ARM_NOP(address) *(vu32*)(address) = 0xE1A00000
#define SAFE_ARM_NOP(address) SAFE_WRITE32(address, 0xE1A00000)
#define ARM_RET(address) *(vu32*)(address) = 0xE12FFF1E // bx lr
#define ARM_NO_COND(address) *(vu32*)(address) = (*(vu32*)(address) & 0x0FFFFFFF) | 0xE0000000

#define SET_BITS(b, p, n, v) \
  ((b) = ((b) & ~(((1u << (n)) - 1) << (p))) | ((v) << (p)))
#define GET_BITS(b, p, n) (((b) >> (p)) & ((1u << (n)) - 1))

#define SIZE(x) ((sizeof(x)) / (sizeof(x[0])))

extern "C" s32 svcInvalidateEntireInstructionCache();

struct Vec2 {
  f32 x, y;

  Vec2() : x(0), y(0) {
  }

  Vec2(f32 x0, f32 y0) : x(x0), y(y0) {
  }
};

struct Vec3 {
  f32 x, y, z;

  Vec3() : x(0), y(0), z(0) {
  }

  Vec3(f32 x0, f32 y0, f32 z0) : x(x0), y(y0), z(z0) {
  }

  static float Dot(const Vec3& a, const Vec3& b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
  }
};

struct Vec4 {
  f32 x, y, z, w;

  Vec4() : x(0), y(0), z(0), w(0) {
  }

  Vec4(f32 x0, f32 y0, f32 z0, f32 w0) : x(x0), y(y0), z(z0), w(w0) {
  }
};

struct Color {
  f32 r, g, b, a;

  Color() : r(1), g(1), b(1), a(1) {
  }

  Color(f32 r0, f32 g0, f32 b0, f32 a0) : r(r0), g(g0), b(b0), a(a0) {
  }
};

struct Color8 {
  u8 r, g, b, a;

  Color8() : r(255), g(255), b(255), a(255) {
  }

  Color8(u8 r0, u8 g0, u8 b0, u8 a0) : r(r0), g(g0), b(b0), a(a0) {
  }

  u32 GetRaw() {
    return READ32((uptr)this);
  }
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

  INLINE c16* GetBuffer() const {
    return buffer;
  }

  void* vtable;
  c16* buffer;
  u16 capacity;
  u16 size;
  bool is_initialized;
};

struct ResourcePack {
  u16 signature; // "PC"
  u16 file_count;
  u32 file_offset[];

  u32 GetSize(u32 idx) const {
    u32 safe_idx = (idx >= file_count) ? 0 : idx;
    return file_offset[safe_idx + 1] - file_offset[safe_idx];
  }

  uptr GetResource(u32 idx) {
    u32 safe_idx = (idx >= file_count) ? 0 : idx;
    return ((uptr)this + file_offset[safe_idx]);
  }
};

struct PokeInfo {
  u16 species;
  u8 form;
  u8 gender;
  bool is_shiny;
  bool is_egg;
  u32 _0;
};