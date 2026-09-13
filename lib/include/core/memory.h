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

#include <3ds.h>
#include "core/types.h"
#include "core/singleton.h"

extern "C" s32 svcInvalidateEntireInstructionCache();

namespace core {

class MemoryManager {
  MAKE_SINGLETON(MemoryManager);

public:
  static bool ToggleProtection(u32 address, bool on) {
    u32 pID;
    if (R_FAILED(svcGetProcessId(&pID, CUR_PROCESS_HANDLE))) {
      return false;
    }

    Handle processHandle;
    if (R_FAILED(svcOpenProcess(&processHandle, pID))) {
      return false;
    }

    MemInfo mInfo;
    PageInfo pInfo;
    if (R_FAILED(svcQueryMemory(&mInfo, &pInfo, address))) {
      svcCloseHandle(processHandle);
      return false;
    }

    MemPerm perm = on
                     ? MemPerm(MEMPERM_READ | MEMPERM_EXECUTE | MEMPERM_WRITE)
                     : MemPerm(MEMPERM_READ | MEMPERM_EXECUTE);

    Result res = svcControlProcessMemory(processHandle, mInfo.base_addr, 0,
                                         mInfo.size, MemOp(MEMOP_PROT), perm);

    svcCloseHandle(processHandle);
    return R_SUCCEEDED(res);
  }

  static bool Unprotect(u32 address, u32 size) {
    return ToggleProtection(address, true);
  }

  static bool Protect(u32 address, u32 size) {
    return ToggleProtection(address, false);
  }
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

// Typed memory access helpers
template <typename T>
INLINE T Read(u32 address) {
  return *reinterpret_cast<volatile T*>(address);
}

template <typename T>
INLINE void Write(u32 address, T value) {
  *reinterpret_cast<volatile T*>(address) = value;
}

template <typename T>
INLINE void SafeWrite(u32 address, T value) {
  MemoryManager::ToggleProtection(address, true);
  *reinterpret_cast<volatile T*>(address) = value;
  MemoryManager::ToggleProtection(address, false);
}

template <typename T>
INLINE T SafeRead(u32 address) {
  MemoryManager::ToggleProtection(address, true);
  T val = *reinterpret_cast<volatile T*>(address);
  MemoryManager::ToggleProtection(address, false);
  return val;
}

INLINE void ArmNop(u32 address) {
  Write<u32>(address, 0xE1A00000);
}

INLINE void ArmReturn(u32 address) {
  Write<u32>(address, 0xE12FFF1E); // bx lr
}

INLINE void ArmReturnTrue(u32 address) {
  Write<u32>(address, 0xE3A00001);     // mov r0, #1
  Write<u32>(address + 4, 0xE12FFF1E); // bx lr
}

INLINE void ArmNoCond(u32 address) {
  Write<u32>(address, (Read<u32>(address) & 0x0FFFFFFF) | 0xE0000000);
}

} // namespace core

using MemoryManager = core::MemoryManager;
using MemoryRange = core::MemoryRange;

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define MEMORY_SCOPE(addr, size) ::core::MemoryRange CONCAT(mem_scope_, __COUNTER__)(addr, size)

#define UNPROTECT(address) ::core::MemoryManager::ToggleProtection((address), true)
#define PROTECT(address)   ::core::MemoryManager::ToggleProtection((address), false)

#define WRITE64(address, value) *(vu64*)(address) = (value)
#define WRITE32(address, value) *(vu32*)(address) = (value)
#define WRITE16(address, value) *(vu16*)(address) = (value)
#define WRITE8(address, value)  *(vu8*)(address)  = (value)
#define WRITEF(address, value)  *(vf32*)(address) = (value)
#define WRITEB(address, value)  *(volatile bool*)(address) = (value)

#define READ(type, address)     *(type*)(address)
#define READ64(address)         *(vu64*)(address)
#define READ32(address)         *(vu32*)(address)
#define READ16(address)         *(vu16*)(address)
#define READ8(address)          *(vu8*)(address)
#define READF(address)          *(vf32*)(address)
#define READB(address)          *(volatile bool*)(address)

#define SAFE_WRITE64(address, value) ::core::SafeWrite<u64>(address, value)
#define SAFE_WRITE32(address, value) ::core::SafeWrite<u32>(address, value)
#define SAFE_WRITE16(address, value) ::core::SafeWrite<u16>(address, value)
#define SAFE_WRITE8(address, value)  ::core::SafeWrite<u8>(address, value)
#define SAFE_WRITEF(address, value)  ::core::SafeWrite<f32>(address, value)
#define SAFE_WRITEB(address, value)  ::core::SafeWrite<bool>(address, value)

#define SAFE_READ64(address, result) do { (result) = ::core::SafeRead<u64>(address); } while(0)
#define SAFE_READ32(address, result) do { (result) = ::core::SafeRead<u32>(address); } while(0)
#define SAFE_READ16(address, result) do { (result) = ::core::SafeRead<u16>(address); } while(0)
#define SAFE_READ8(address, result)  do { (result) = ::core::SafeRead<u8>(address); } while(0)
#define SAFE_READF(address, result)  do { (result) = ::core::SafeRead<f32>(address); } while(0)
#define SAFE_READB(address, result)  do { (result) = ::core::SafeRead<bool>(address); } while(0)

#define ARM_NOP(address)         ::core::ArmNop(address)
#define ARM_RETURN_TRUE(address) ::core::ArmReturnTrue(address)
#define SAFE_ARM_NOP(address)    ::core::SafeWrite<u32>(address, 0xE1A00000)
#define ARM_RET(address)         ::core::ArmReturn(address)
#define ARM_NO_COND(address)     ::core::ArmNoCond(address)
