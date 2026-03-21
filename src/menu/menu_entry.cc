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

#include "menu/menu_entry.h"

#include "menu/plugin_menu.h"
#include "utils.h"

namespace menu {

/**
 * @brief Helper to set specific bits in memory.
 */
static void SetBits(u32 *num, u32 offset, u32 size, u32 value) {
  num += size / 32;
  size %= 32;
  value %= 1u << size;
  SET_BITS(*num, offset, size, value);
}

MenuEntry::MenuEntry()
    : name_(nullptr),
      address_(nullptr),
      array_(nullptr),
      type_(kTypeMax),
      bit_offset_(0),
      bit_size_(0),
      array_size_(0) {}

void MenuEntry::Initialize(const c8 *name, void *addr, u8 type, u32 bit_offset,
                           u32 bit_size) {
  name_ = name;
  address_ = addr;
  type_ = type;
  bit_offset_ = bit_offset;
  bit_size_ = bit_size;
  array_ = nullptr;
  callback_ = nullptr;
  array_size_ = 0;
}

MenuEntry &MenuEntry::WithArray(const c8 *array[], u32 array_size) {
  array_ = array;
  array_size_ = array_size;
  return *this;
}

MenuEntry &MenuEntry::WithCallback(callback_t callback) {
  callback_ = callback;
  return *this;
}

u8 MenuEntry::GetType() const { return type_; }

void MenuEntry::GetDefaultDisplayValue(c16 *buffer) const {
  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      Utils::Format(buffer, u"%s : %d", name_, *(u8 *)address_);
      break;
    case kTypeU16:
    case kTypeS16:
      Utils::Format(buffer, u"%s : %d", name_, *(u16 *)address_);
      break;
    case kTypePointer:
    case kTypeU32:
    case kTypeS32:
      Utils::Format(buffer, u"%s : %d", name_, *(u32 *)address_);
      break;
    case kTypeU64:
    case kTypeS64:
      Utils::Format(buffer, u"%s : 0x%016llx", name_, *(u64 *)address_);
      break;
    case kTypeF32:
      Utils::Format(buffer, u"%s : %.2f", name_, *(f32 *)address_);
      break;
    case kTypeF64:
      Utils::Format(buffer, u"%s : %.2f", name_, *(f64 *)address_);
      break;
    case kTypeBits:
      Utils::Format(buffer, u"%s : %d", name_,
                    GET_BITS(*(u32 *)address_, bit_offset_, bit_size_));
      break;
    case kTypeBoolean:
      Utils::Format(buffer, u"%s : %ls", name_,
                    *(bool *)address_ ? u"True" : u"False");
      break;
    case kTypeCheatCode:
      Utils::Format(buffer, u"%s : %ls", name_, u"Off");
      break;
    case kTypeUnicode:
      Utils::Format(buffer, u"%s : \"%ls\"", name_, (const c16 *)address_);
      break;
    default:
      Utils::Format(buffer, u"%s : ???", name_);
      break;
  }
}

void MenuEntry::GetArrayDisplayValue(c16 *buffer) const {
  u32 index = 0;

  switch (type_) {
    case kTypeU8:
      index = *(u8 *)address_;
      break;
    case kTypeS8:
      index = *(s8 *)address_;
      break;
    case kTypeU16:
      index = *(u16 *)address_;
      break;
    case kTypeS16:
      index = *(s16 *)address_;
      break;
    case kTypeU32:
      index = *(u32 *)address_;
      break;
    case kTypeS32:
      index = *(s32 *)address_;
      break;
    case kTypeU64:
      index = *(u64 *)address_;
      break;
    case kTypeS64:
      index = *(s64 *)address_;
      break;
    case kTypePointer:
      index = *(u32 *)address_;
      break;
    case kTypeBits:
      index = GET_BITS(*(u32 *)address_, bit_offset_, bit_size_);
      break;
    case kTypeBoolean:
      index = *(bool *)address_;
      break;
    default:
      Utils::Format(buffer, u"%s : ???", name_);
      return;
  }

  index %= array_size_;
  Utils::Format(buffer, u"%s : <%s> [%d/%d]", name_, array_[index], index + 1,
                array_size_);
}

void MenuEntry::GetDisplayValue(c16 *buffer) const {
  if (type_ == kTypeMenu) {
    Utils::Format(buffer, u"[%s]", name_);
    return;
  }
  if (array_size_ != 0) {
    GetArrayDisplayValue(buffer);
    return;
  }
  GetDefaultDisplayValue(buffer);
}

void MenuEntry::Increment(u32 count) {
  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      *(u8 *)address_ += count;
      break;
    case kTypeU16:
    case kTypeS16:
      *(u16 *)address_ += count;
      break;
    case kTypePointer:
    case kTypeU32:
    case kTypeS32:
      *(u32 *)address_ += count;
      break;
    case kTypeU64:
    case kTypeS64:
      *(u64 *)address_ += count;
      break;
    case kTypeF32:
      *(f32 *)address_ += count;
      break;
    case kTypeF64:
      *(f64 *)address_ += count;
      break;
    case kTypeBits: {
      u32 b = GET_BITS(*(u32 *)address_, bit_offset_, bit_size_);
      b += count;
      SetBits((u32 *)address_, bit_offset_, bit_size_, b);
    } break;
    case kTypeBoolean:
      *(bool *)address_ ^= true;
      break;
    case kTypeCheatCode:
      // ((CheatCode *)address_)->Toggle();
      break;
    default:
      break;
  }
}

void MenuEntry::Decrement(u32 count) {
  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      *(u8 *)address_ -= count;
      break;
    case kTypeU16:
    case kTypeS16:
      *(u16 *)address_ -= count;
      break;
    case kTypePointer:
    case kTypeU32:
    case kTypeS32:
      *(u32 *)address_ -= count;
      break;
    case kTypeU64:
    case kTypeS64:
      *(u64 *)address_ -= count;
      break;
    case kTypeF32:
      *(f32 *)address_ -= count;
      break;
    case kTypeF64:
      *(f64 *)address_ -= count;
      break;
    case kTypeBits: {
      u32 b = GET_BITS(*(u32 *)address_, bit_offset_, bit_size_);
      b -= count;
      SetBits((u32 *)address_, bit_offset_, bit_size_, b);
    } break;
    case kTypeBoolean:
      *(bool *)address_ ^= true;
      break;
    case kTypeCheatCode:
      // ((CheatCode *)address_)->Toggle();
      break;
    default:
      break;
  }
}

void MenuEntry::Edit(const void *value) {
  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      *(u8 *)address_ = *(u8 *)value;
      break;
    case kTypeU16:
    case kTypeS16:
      *(u16 *)address_ = *(u16 *)value;
      break;
    case kTypePointer:
    case kTypeU32:
    case kTypeS32:
      *(u32 *)address_ = *(u32 *)value;
      break;
    case kTypeMenu:
      PluginMenu::GetInstance().EnterSubMenu((menu_callback_t)address_);
      break;
    case kTypeBoolean:
      *(bool *)address_ = *(bool *)value;
      break;
    case kTypeUnicode:
      for (u32 i = 0; i < bit_offset_; i++) {
        *(c16 *)((uptr)address_ + i * 2) = *(c16 *)((uptr)value + i * 2);
      }
      *(c16 *)((uptr)address_ + (bit_offset_ - 1) * 2) = 0;
      break;
    default:
      break;
  }
}

void MenuEntry::Execute(void *args) {
  if (callback_ != nullptr) callback_(args);
}

}  // namespace menu