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

#include "hack/cheat_code.h"
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
      array_size_(0),
      refresh_(0),
      min_(0),
      max_(0),
      is_min_used_(0),
      is_max_used_(0) {}

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
  refresh_ = 0;
  min_ = 0;
  max_ = 0;
  is_min_used_ = 0;
  is_max_used_ = 0;
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

MenuEntry &MenuEntry::WithRefresh() {
  refresh_ = 1;
  return *this;
}

MenuEntry &MenuEntry::WithMin(s32 min) {
  min_ = min;
  is_min_used_ = 1;
  return *this;
}

MenuEntry &MenuEntry::WithMax(s32 max) {
  max_ = max;
  is_max_used_ = 1;
  return *this;
}

MenuEntry &MenuEntry::WithArgs(void *args) {
  args_ = args;
  return *this;
}

u8 MenuEntry::GetType() const { return type_; }

void MenuEntry::GetDefaultDisplayValue(c16 *buffer) const {
  switch (type_) {
    case kTypeU8:
      Utils::Format(buffer, u"%s : %u", name_, *(u8 *)address_);
      break;

    case kTypeS8:
      Utils::Format(buffer, u"%s : %d", name_, *(s8 *)address_);
      break;

    case kTypeU16:
      Utils::Format(buffer, u"%s : %u", name_, *(u16 *)address_);
      break;

    case kTypeS16:
      Utils::Format(buffer, u"%s : %d", name_, *(s16 *)address_);
      break;

    case kTypePointer:
      Utils::Format(buffer, u"%s : 0x%08X", name_, address_);
      break;

    case kTypeU32:
      Utils::Format(buffer, u"%s : %u", name_, *(u32 *)address_);
      break;

    case kTypeS32:
      Utils::Format(buffer, u"%s : %d", name_, *(s32 *)address_);
      break;

    case kTypeU64:
      Utils::Format(buffer, u"%s : %llu", name_, *(u64 *)address_);
      break;

    case kTypeS64:
      Utils::Format(buffer, u"%s : %lld", name_, *(s64 *)address_);
      break;

    case kTypeF32:
      Utils::Format(buffer, u"%s : %.2f", name_, *(f32 *)address_);
      break;

    case kTypeF64:
      Utils::Format(buffer, u"%s : %.2f", name_, *(f64 *)address_);
      break;

    case kTypeBits:
      Utils::Format(buffer, u"%s : %u", name_,
                    GET_BITS(*(u32 *)address_, bit_offset_, bit_size_));
      break;

    case kTypeBoolean:
      Utils::Format(buffer, u"%s : %ls", name_,
                    *(bool *)address_ ? u"True" : u"False");
      break;

    case kTypeCheatCode:
      Utils::Format(buffer, u"%s : %s", name_,
                    ((CheatCode *)address_)->IsEnabled() ? "On" : "Off");
      break;

    case kTypeUnicode:
      Utils::Format(buffer, u"%s : \"%ls\"", name_, address_);
      break;

    case kTypeIdle:
      Utils::Format(buffer, u"%s", name_);
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
#define INCREMENT_WRAP(type)                              \
  {                                                       \
    type val = *(type *)address_;                         \
    /* Si on va dépasser le max, on revient au min */     \
    if (is_max_used_ && ((s32)val + (s32)count > max_)) { \
      val = (type)min_;                                   \
    } else {                                              \
      val += (type)count;                                 \
    }                                                     \
    *(type *)address_ = val;                              \
  }

  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      INCREMENT_WRAP(u8);
      break;
    case kTypeU16:
    case kTypeS16:
      INCREMENT_WRAP(u16);
      break;
    case kTypeU32:
    case kTypeS32:
    case kTypePointer:
      INCREMENT_WRAP(u32);
      break;

    case kTypeU64:
    case kTypeS64:
      *(u64 *)address_ += count;
      break;
    case kTypeF32:
      *(f32 *)address_ += (f32)count;
      break;
    case kTypeF64:
      *(f64 *)address_ += (f64)count;
      break;

    case kTypeBits: {
      u32 b = GET_BITS(*(u32 *)address_, bit_offset_, bit_size_) + count;
      if (is_max_used_ && (s32)b > max_) b = (u32)min_;
      SetBits((u32 *)address_, bit_offset_, bit_size_, b);
    } break;

    case kTypeBoolean:
      *(bool *)address_ ^= true;
      break;

    case kTypeCheatCode:
      ((CheatCode *)address_)->Toggle();
      break;

    default:
      break;
  }

#undef INCREMENT_WRAP
  if (refresh_) PluginMenu::GetInstance().Refresh();
}

void MenuEntry::Decrement(u32 count) {
#define DECREMENT_WRAP(type)                                 \
  {                                                          \
    type val = *(type *)address_;                            \
    /* Si on va descendre en dessous du min, on va au max */ \
    if (is_min_used_ && ((s32)val - (s32)count < min_)) {    \
      val = (type)max_;                                      \
    } else {                                                 \
      val -= (type)count;                                    \
    }                                                        \
    *(type *)address_ = val;                                 \
  }

  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      DECREMENT_WRAP(u8);
      break;
    case kTypeU16:
    case kTypeS16:
      DECREMENT_WRAP(u16);
      break;
    case kTypeU32:
    case kTypeS32:
    case kTypePointer:
      DECREMENT_WRAP(u32);
      break;

    case kTypeU64:
    case kTypeS64:
      *(u64 *)address_ -= count;
      break;
    case kTypeF32:
      *(f32 *)address_ -= (f32)count;
      break;
    case kTypeF64:
      *(f64 *)address_ -= (f64)count;
      break;

    case kTypeBits: {
      u32 b = GET_BITS(*(u32 *)address_, bit_offset_, bit_size_) - count;
      if (is_min_used_ && (s32)b < min_) b = (u32)max_;
      SetBits((u32 *)address_, bit_offset_, bit_size_, b);
    } break;

    case kTypeBoolean:
      *(bool *)address_ ^= true;
      break;

    case kTypeCheatCode:
      ((CheatCode *)address_)->Toggle();
      break;

    default:
      break;
  }

#undef DECREMENT_WRAP
  if (refresh_) PluginMenu::GetInstance().Refresh();
}

void MenuEntry::Edit(const void *value) {
#define EDIT_CLAMP(type)                                   \
  {                                                        \
    type val = *(type *)value;                             \
    if (is_min_used_ && (s32)val < min_) val = (type)min_; \
    if (is_max_used_ && (s32)val > max_) val = (type)max_; \
    *(type *)address_ = val;                               \
  }

  switch (type_) {
    case kTypeU8:
    case kTypeS8:
      EDIT_CLAMP(u8);
      break;
    case kTypeU16:
    case kTypeS16:
      EDIT_CLAMP(u16);
      break;
    case kTypeU32:
    case kTypeS32:
    case kTypePointer:
      EDIT_CLAMP(u32);
      break;

    case kTypeBoolean:
      *(bool *)address_ = *(bool *)value;
      break;

    case kTypeMenu:
      PluginMenu::GetInstance().EnterSubMenu((menu_callback_t)address_, args_);
      break;

    case kTypeUnicode:
      // bit_offset_ est utilisé ici comme taille max du buffer
      for (u32 i = 0; i < bit_offset_; i++) {
        *(c16 *)((uptr)address_ + i * 2) = *(c16 *)((uptr)value + i * 2);
      }
      // Garanti la terminaison nulle
      *(c16 *)((uptr)address_ + (bit_offset_ - 1) * 2) = 0;
      break;

    case kTypeCheatCode:
      ((CheatCode *)address_)->Toggle();
      break;

    default:
      break;
  }

#undef EDIT_CLAMP
  if (refresh_) PluginMenu::GetInstance().Refresh();
}

void MenuEntry::Execute() {
  if (kTypeMenu == type_) {
    PluginMenu::GetInstance().EnterSubMenu((menu_callback_t)address_, args_);
  } else if (callback_ != nullptr) {
    callback_(args_);
  }
}

}  // namespace menu