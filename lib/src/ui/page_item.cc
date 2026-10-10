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
 * @file page_item.cc
 * @brief One entry of a menu page. See ui/page_item.h.
 */

#include "ui/page_item.h"

#include "core/cheat_code.h"
#include "ui/main_application.h"
#include "core/utils.h"
#include <cstring>
#include "pokemon/native/item_data.h"
#include "ui/name_list.h"

namespace ui {
static void SetBits(u32* num, u32 offset, u32 size, u32 value) {
  num += size / 32;
  size %= 32;
  value %= 1u << size;
  SET_BITS(*num, offset, size, value);
}

/// Adds `count` to the value at `address`. When the result is more than `max`,
/// the value goes back to `min`. The maximum applies only when it is set.
template <typename T>
static void IncrementWrapped(void* address, s32 min, s32 max,
                             bool is_max_used, u32 count) {
  T val = *(T*)address;
  if (is_max_used && ((s32)val + (s32)count > max)) {
    val = (T)min;
  } else {
    val += (T)count;
  }
  *(T*)address = val;
}

/// Subtracts `count` from the value at `address`. When the result is less than
/// `min`, the value goes to `max`. The minimum applies only when it is set.
template <typename T>
static void DecrementWrapped(void* address, s32 min, s32 max,
                             bool is_min_used, u32 count) {
  T val = *(T*)address;
  if (is_min_used && ((s32)val - (s32)count < min)) {
    val = (T)max;
  } else {
    val -= (T)count;
  }
  *(T*)address = val;
}

/// Writes `value` to `address`, between `min` and `max`. Each limit applies
/// only when it is set.
template <typename T>
static void EditClamped(const void* value, void* address, s32 min, s32 max,
                        bool is_min_used, bool is_max_used) {
  T val = *(const T*)value;
  if (is_min_used && (s32)val < min) val = (T)min;
  if (is_max_used && (s32)val > max) val = (T)max;
  *(T*)address = val;
}

/// Reads a number that the player typed. Returns false when the text has no
/// digit. `integer` gets the whole part, `decimal` gets the full value.
static bool ParseNumber(const c16* text, s64& integer, f64& decimal) {
  const bool is_negative = *text == u'-';
  if (is_negative) text++;
  u32 base = 10;
  if (text[0] == u'0' && (text[1] == u'x' || text[1] == u'X')) {
    base = 16;
    text += 2;
  }

  u64 whole = 0;
  f64 fraction = 0.0;
  f64 scale = 0.1;
  bool has_digit = false;
  bool has_dot = false;
  for (; *text != 0; text++) {
    const c16 c = *text;
    u32 digit;
    if (c == u'.' && base == 10 && !has_dot) {
      has_dot = true;
      continue;
    }
    if (c >= u'0' && c <= u'9') {
      digit = c - u'0';
    } else if (base == 16 && c >= u'a' && c <= u'f') {
      digit = 10 + (c - u'a');
    } else if (base == 16 && c >= u'A' && c <= u'F') {
      digit = 10 + (c - u'A');
    } else {
      break;
    }
    has_digit = true;
    if (has_dot) {
      fraction += digit * scale;
      scale /= 10.0;
    } else {
      whole = whole * base + digit;
    }
  }

  integer = is_negative ? -(s64)whole : (s64)whole;
  decimal = ((f64)whole + fraction) * (is_negative ? -1.0 : 1.0);
  return has_digit;
}

/// Writes `value` to `address`, between `min` and `max`. Each limit applies
/// only when it is set.
template <typename T>
static void StoreClamped(void* address, s64 value, s32 min, s32 max,
                         bool is_min_used, bool is_max_used) {
  if (is_min_used && value < min) value = min;
  if (is_max_used && value > max) value = max;
  *(T*)address = (T)value;
}

/// Removes the zeros at the end of a decimal number ("1.500" -> "1.5").
static void TrimZeros(c16* buffer) {
  c16* dot = nullptr;
  c16* end = buffer;
  for (; *end != 0; end++) {
    if (*end == u'.') dot = end;
  }
  if (dot == nullptr) return;
  while (end > dot + 1 && end[-1] == u'0') end--;
  if (end == dot + 1) end--;
  *end = 0;
}

PageItem::PageItem()
  : name_(nullptr),
    address_(nullptr),
    array_(nullptr),
    type_(kTypeMax),
    bit_offset_(0),
    bit_size_(0),
    description_(nullptr),
    array_size_(0),
    refresh_(0),
    min_(0),
    max_(0),
    is_min_used_(0),
    factor_(1.0f),
    is_max_used_(0),
    is_read_only_(0),
    needs_confirm_(0),
    icon_kind_(IconKind::kNone),
    icon_ids_(nullptr),
    suggest_(nullptr) {
}

void PageItem::Initialize(const c8* name, void* addr, u8 type, u32 bit_offset,
                          u32 bit_size) {
  name_ = name;
  address_ = addr;
  type_ = type;
  bit_offset_ = bit_offset;
  bit_size_ = bit_size;
  array_ = nullptr;
  callback_ = nullptr;
  args_ = nullptr;
  description_ = nullptr;
  array_size_ = 0;
  refresh_ = 0;
  min_ = 0;
  max_ = 0;
  is_min_used_ = 0;
  factor_ = 1.0f;
  is_max_used_ = 0;
  is_read_only_ = 0;
  needs_confirm_ = 0;
  icon_kind_ = IconKind::kNone;
  icon_ids_ = nullptr;
  suggest_ = nullptr;
}

PageItem& PageItem::WithArray(const c8* array[], u32 array_size) {
  array_ = array;
  array_size_ = array_size;
  WithMin(0);
  WithMax(array_size - 1);
  return *this;
}

PageItem& PageItem::WithCallback(callback_t callback) {
  callback_ = callback;
  return *this;
}

PageItem& PageItem::WithRefresh() {
  refresh_ = 1;
  return *this;
}

PageItem& PageItem::WithMin(s32 min) {
  min_ = min;
  is_min_used_ = 1;
  return *this;
}

PageItem& PageItem::WithMax(s32 max) {
  max_ = max;
  is_max_used_ = 1;
  return *this;
}

PageItem& PageItem::WithArgs(void* args) {
  args_ = args;
  return *this;
}

PageItem& PageItem::WithFactor(f32 factor) {
  factor_ = factor;
  return *this;
}

PageItem& PageItem::WithDescription(const c8* description) {
  description_ = description;
  return *this;
}

PageItem& PageItem::WithIcons(IconKind kind, const u16* ids) {
  icon_kind_ = kind;
  icon_ids_ = ids;
  return *this;
}

PageItem& PageItem::WithSuggestions(suggest_t suggest) {
  suggest_ = suggest;
  return *this;
}

PageItem& PageItem::WithConfirm() {
  needs_confirm_ = 1;
  return *this;
}

bool PageItem::IsInteger() const {
  switch (type_) {
    case kTypeU8:
    case kTypeS8:
    case kTypeU16:
    case kTypeS16:
    case kTypeU32:
    case kTypeS32:
    case kTypeBits:
    case kTypeSpecies:
    case kTypeAbility:
    case kTypeMove:
    case kTypeItem:
      return HasValue();
    default:
      return false;
  }
}

bool PageItem::GetRange(s64& min, s64& max) const {
  if (!IsInteger()) return false;
  switch (type_) {
    case kTypeS8:
      min = -128;
      max = 127;
      break;
    case kTypeS16:
      min = -32768;
      max = 32767;
      break;
    case kTypeS32:
      min = -2147483648ll;
      max = 2147483647ll;
      break;
    case kTypeU32:
      min = 0;
      max = 0xFFFFFFFFll;
      break;
    case kTypeBits:
      min = 0;
      max = bit_size_ >= 32 ? 0xFFFFFFFFll : (1ll << bit_size_) - 1;
      break;
    case kTypeU16:
    case kTypeSpecies:
    case kTypeMove:
    case kTypeItem:
      min = 0;
      max = 0xFFFF;
      break;
    default:
      min = 0;
      max = 0xFF;
      break;
  }
  if (is_min_used_ && min_ > min) min = min_;
  if (is_max_used_ && max_ < max) max = max_;
  if (max < min) max = min;
  return true;
}

s64 PageItem::ReadNumber() const {
  switch (type_) {
    case kTypeS8:
      return *(s8*)address_;
    case kTypeS16:
      return *(s16*)address_;
    case kTypeS32:
      return *(s32*)address_;
    case kTypeU32:
      return *(u32*)address_;
    default:
      return (s64)ReadValue();
  }
}

u64 PageItem::ReadValue() const {
  if (!HasValue()) return 0;
  if (type_ == kTypeBits) {
    return GET_BITS(*(u32*)address_, bit_offset_, bit_size_);
  }
  if (type_ == kTypeCheatCode) {
    return ((core::CheatCode*)address_)->IsEnabled() ? 1 : 0;
  }
  const u32 size = GetValueSize();
  if (size <= sizeof(u64)) {
    u64 value = 0;
    memcpy(&value, address_, size);
    return value;
  }
  // A text: a hash of its bytes (FNV-1a).
  u64 hash = 14695981039346656037ull;
  const u8* bytes = (const u8*)address_;
  for (u32 i = 0; i < size; i++) {
    hash = (hash ^ bytes[i]) * 1099511628211ull;
  }
  return hash;
}

bool PageItem::CanWriteValue() const {
  return HasValue() && type_ != kTypeCheatCode &&
         GetValueSize() <= sizeof(u64);
}

void PageItem::WriteValue(u64 value) {
  if (!CanWriteValue()) return;
  if (type_ == kTypeBits) {
    SetBits((u32*)address_, bit_offset_, bit_size_, (u32)value);
    return;
  }
  memcpy(address_, &value, GetValueSize());
}

PageItem& PageItem::WithReadOnly() {
  is_read_only_ = 1;
  return *this;
}

bool PageItem::IsSigned() const {
  switch (type_) {
    case kTypeS8:
    case kTypeS16:
    case kTypeS32:
    case kTypeS64:
    case kTypeF32:
    case kTypeF64:
      return true;
    default:
      return is_min_used_ && min_ < 0;
  }
}

bool PageItem::IsDecimal() const {
  return type_ == kTypeF32 || type_ == kTypeF64;
}

bool PageItem::HasValue() const {
  switch (type_) {
    case kTypeMenu:
    case kTypeIdle:
    case kTypeSeparator:
      return false;
    default:
      return address_ != nullptr;
  }
}

s32 PageItem::GetIndex() const {
  switch (type_) {
    case kTypeU8:
    case kTypeAbility:
      return *(u8*)address_;
    case kTypeS8:
      return *(s8*)address_;
    case kTypeU16:
    case kTypeSpecies:
    case kTypeMove:
    case kTypeItem:
      return *(u16*)address_;
    case kTypeS16:
      return *(s16*)address_;
    case kTypeU32:
    case kTypePointer:
      return *(u32*)address_;
    case kTypeS32:
      return *(s32*)address_;
    case kTypeU64:
      return (s32)*(u64*)address_;
    case kTypeS64:
      return (s32)*(s64*)address_;
    case kTypeBits:
      return GET_BITS(*(u32*)address_, bit_offset_, bit_size_);
    case kTypeBoolean:
      return *(bool*)address_;
    case kTypeCheatCode:
      return ((core::CheatCode*)address_)->IsEnabled();
    default:
      return 0;
  }
}

bool PageItem::IsSameAs(const PageItem& other) const {
  // A std::function cannot be compared: the name tells the actions apart.
  if (type_ != other.type_ || address_ != other.address_ ||
      args_ != other.args_ ||
      (callback_ == nullptr) != (other.callback_ == nullptr)) {
    return false;
  }
  if (name_ == other.name_) return true;
  if (name_ == nullptr || other.name_ == nullptr) return false;
  return strcmp(name_, other.name_) == 0;
}

u8 PageItem::GetType() const { return type_; }

void PageItem::GetValueText(c16* buffer) const {
  buffer[0] = 0;
  if (array_size_ != 0 && HasValue()) {
    const u32 index = (u32)GetIndex() % array_size_;
    core::Utils::Format(buffer, u"< %s >", array_[index]);
    return;
  }

  switch (type_) {
    case kTypeU8:
      core::Utils::Format(buffer, u"%u", *(u8*)address_);
      break;
    case kTypeS8:
      core::Utils::Format(buffer, u"%d", *(s8*)address_);
      break;
    case kTypeU16:
      core::Utils::Format(buffer, u"%u", *(u16*)address_);
      break;
    case kTypeS16:
      core::Utils::Format(buffer, u"%d", *(s16*)address_);
      break;
    case kTypePointer:
      core::Utils::Format(buffer, u"0x%08X", address_);
      break;
    case kTypeU32:
      core::Utils::Format(buffer, u"%u", *(u32*)address_);
      break;
    case kTypeS32:
      core::Utils::Format(buffer, u"%d", *(s32*)address_);
      break;
    case kTypeU64:
      core::Utils::Format(buffer, u"%llu", *(u64*)address_);
      break;
    case kTypeS64:
      core::Utils::Format(buffer, u"%lld", *(s64*)address_);
      break;
    case kTypeF32:
      core::Utils::Format(buffer, u"%.2f", *(f32*)address_);
      break;
    case kTypeF64:
      core::Utils::Format(buffer, u"%.2f", *(f64*)address_);
      break;
    case kTypeBits:
      core::Utils::Format(buffer, u"%u",
                          GET_BITS(*(u32*)address_, bit_offset_, bit_size_));
      break;
    case kTypeBoolean:
      core::Utils::Format(buffer, u"%s", *(bool*)address_ ? "On" : "Off");
      break;
    case kTypeCheatCode:
      core::Utils::Format(
          buffer, u"%s",
          ((core::CheatCode*)address_)->IsEnabled() ? "On" : "Off");
      break;
    case kTypeUnicode: {
      c16 text[64];
      const u32 length = (bit_offset_ < 64) ? bit_offset_ : 63;
      for (u32 i = 0; i < length; i++) {
        text[i] = ((c16*)address_)[i];
      }
      text[length] = 0;
      core::Utils::Format(buffer, u"\"%ls\"", text);
      break;
    }
    case kTypeAbility:
      ((void (*)(String*, u8))pokemon::address::kGetAbilityName)(
          String::GetTmpStr(), *(u8*)address_);
      core::Utils::Format(buffer, u"%ls", String::GetTmpBuf());
      break;
    case kTypeSpecies:
      ((void (*)(String*, u16))pokemon::address::kGetSpeciesName)(
          String::GetTmpStr(), *(u16*)address_);
      core::Utils::Format(buffer, u"N°%03d %ls", *(u16*)address_,
                          String::GetTmpBuf());
      break;
    case kTypeMove:
      ((void (*)(u16, String*))pokemon::address::kGetMoveName)(
          *(u16*)address_, String::GetTmpStr());
      core::Utils::Format(buffer, u"%ls", String::GetTmpBuf());
      break;
    case kTypeItem: {
      // The names that the menu keeps: the name of an item from the game
      // reads two files, and the menu draws the value in each frame.
      c16 name[64];
      NameList::GetName(*this, *(u16*)address_, name, SIZE(name));
      core::Utils::Format(buffer, u"%ls", name);
      break;
    }
    case kTypeMenu:
      core::Utils::Format(buffer, u">");
      break;
    default:
      break;
  }
}

void PageItem::GetNameText(c16* buffer) const {
  // An entry that runs a function with A starts with the icon of the A
  // button.
  const c16* prefix =
      (callback_ == nullptr || type_ == kTypeMenu) ? u"" : u" ";
  core::Utils::Format(buffer, u"%ls%s", prefix,
                      name_ != nullptr ? name_ : "");
}

void PageItem::GetDisplayValue(c16* buffer) const {
  if (type_ == kTypeMenu) {
    core::Utils::Format(buffer, u"[%s]", name_);
    return;
  }
  if (type_ == kTypeSeparator) {
    if (name_ != nullptr && name_[0] != '\0') {
      core::Utils::Format(buffer, u"―― %s ――――",
                          name_);
      return;
    }
    u32 i;
    for (i = 0; i < 16; i++) buffer[i] = 0x2015;
    buffer[i] = 0;
    return;
  }

  c16 name[96];
  GetNameText(name);
  if (type_ == kTypeIdle || !HasValue()) {
    core::Utils::Format(buffer, u"%ls", name);
    return;
  }
  c16 value[96];
  GetValueText(value);
  core::Utils::Format(buffer, u"%ls : %ls", name, value);
}

u32 PageItem::GetValueSize() const {
  if (!HasValue()) return 0;
  switch (type_) {
    case kTypeU8:
    case kTypeS8:
    case kTypeAbility:
    case kTypeBoolean:
      return 1;
    case kTypeU16:
    case kTypeS16:
    case kTypeSpecies:
    case kTypeMove:
    case kTypeItem:
      return 2;
    case kTypeU32:
    case kTypeS32:
    case kTypeF32:
    case kTypePointer:
    case kTypeBits:
      return 4;
    case kTypeU64:
    case kTypeS64:
    case kTypeF64:
      return 8;
    case kTypeUnicode:
      return bit_offset_ * sizeof(c16);
    default:
      return 0;
  }
}

void PageItem::Increment(u32 count) {
  if (is_read_only_) return;
  switch (type_) {
    case kTypeU8:
    case kTypeAbility:
      IncrementWrapped<u8>(address_, min_, max_, is_max_used_, count);
      break;
    case kTypeS8:
      IncrementWrapped<s8>(address_, min_, max_, is_max_used_, count);
      break;
    case kTypeU16:
    case kTypeSpecies:
    case kTypeMove:
    case kTypeItem:
      IncrementWrapped<u16>(address_, min_, max_, is_max_used_, count);
      break;
    case kTypeS16:
      IncrementWrapped<s16>(address_, min_, max_, is_max_used_, count);
      break;
    case kTypeU32:
    case kTypePointer:
      IncrementWrapped<u32>(address_, min_, max_, is_max_used_, count);
      break;
    case kTypeS32:
      IncrementWrapped<s32>(address_, min_, max_, is_max_used_, count);
      break;

    case kTypeU64:
    case kTypeS64:
      *(u64*)address_ += count;
      break;
    case kTypeF32: {
      f32 val = *(f32*)address_ + ((f32)count * factor_);
      if (is_max_used_ && val
          >
          (f32)max_
      ) {
        val = (f32)min_;
      }
      *(f32*)address_ = val;
      break;
    }
    case kTypeF64: {
      f64 val = *(f64*)address_ + ((f64)count * (f64)factor_);
      if (is_max_used_ && val
          >
          max_
      ) {
        val = min_;
      }
      *(f64*)address_ = val;
      break;
    }

    case kTypeBits: {
      u32 b = GET_BITS(*(u32*)address_, bit_offset_, bit_size_) + count;
      if (is_max_used_ && (s32)b > max_) b = (u32)min_;
      SetBits((u32*)address_, bit_offset_, bit_size_, b);
    }
    break;

    case kTypeBoolean:
      *(bool*)address_ ^= true;
      break;

    case kTypeCheatCode:
      ((core::CheatCode*)address_)->Toggle();
      ((core::CheatCode*)address_)->Execute();
      break;

    default:
      break;
  }
}

void PageItem::Decrement(u32 count) {
  if (is_read_only_) return;
  switch (type_) {
    case kTypeU8:
    case kTypeAbility:
      DecrementWrapped<u8>(address_, min_, max_, is_min_used_, count);
      break;
    case kTypeS8:
      DecrementWrapped<s8>(address_, min_, max_, is_min_used_, count);
      break;
    case kTypeU16:
    case kTypeSpecies:
    case kTypeMove:
    case kTypeItem:
      DecrementWrapped<u16>(address_, min_, max_, is_min_used_, count);
      break;
    case kTypeS16:
      DecrementWrapped<s16>(address_, min_, max_, is_min_used_, count);
      break;
    case kTypeU32:
    case kTypePointer:
      DecrementWrapped<u32>(address_, min_, max_, is_min_used_, count);
      break;
    case kTypeS32:
      DecrementWrapped<s32>(address_, min_, max_, is_min_used_, count);
      break;

    case kTypeU64:
    case kTypeS64:
      *(u64*)address_ -= count;
      break;
    case kTypeF32: {
      f32 val = *(f32*)address_ - ((f32)count * factor_);
      if (is_min_used_ && val < (f32)min_) {
        val = (f32)max_;
      }
      *(f32*)address_ = val;
      break;
    }
    case kTypeF64: {
      f64 val = *(f64*)address_ - ((f64)count * (f64)factor_);
      if (is_min_used_ && val < min_) {
        val = max_;
      }
      *(f64*)address_ = val;
      break;
    }

    case kTypeBits: {
      u32 b = GET_BITS(*(u32*)address_, bit_offset_, bit_size_) - count;
      if (is_min_used_ && (s32)b < min_) b = (u32)max_;
      SetBits((u32*)address_, bit_offset_, bit_size_, b);
    }
    break;

    case kTypeBoolean:
      *(bool*)address_ ^= true;
      break;

    case kTypeCheatCode:
      ((core::CheatCode*)address_)->Toggle();
      ((core::CheatCode*)address_)->Execute();
      break;

    default:
      break;
  }
}

void PageItem::Edit(const void* value) {
  if (is_read_only_ && type_ != kTypeMenu) return;
  switch (type_) {
    case kTypeU8:
    case kTypeAbility:
    case kTypeS8:
      EditClamped<u8>(value, address_, min_, max_, is_min_used_,
                      is_max_used_);
      break;
    case kTypeU16:
    case kTypeSpecies:
    case kTypeS16:
    case kTypeMove:
    case kTypeItem:
      EditClamped<u16>(value, address_, min_, max_, is_min_used_,
                       is_max_used_);
      break;
    case kTypeU32:
    case kTypeS32:
    case kTypePointer:
      EditClamped<u32>(value, address_, min_, max_, is_min_used_,
                       is_max_used_);
      break;

    case kTypeF32: {
      f32 val = (f32)(*(u32*)value);
      if (is_min_used_ && val < (f32)min_) val = (f32)min_;
      if (is_max_used_ && val
          >
          (f32)max_
      )
        val = (f32)max_;
      *(f32*)address_ = val;
      break;
    }
    case kTypeF64: {
      f64 val = (f64)(*(u32*)value);
      if (is_min_used_ && val < min_) val = min_;
      if (is_max_used_ && val
          >
          max_
      )
        val = max_;
      *(f64*)address_ = val;
      break;
    }

    case kTypeBoolean:
      *(bool*)address_ ^= true;
      break;

    case kTypeMenu:
      MainApplication::GetInstance().Open((menu_callback_t)address_, args_,
                                          name_);
      break;

    case kTypeUnicode:
      for (u32 i = 0; i < bit_offset_; i++) {
        *(c16*)((uptr)address_ + i * 2) = *(c16*)((uptr)value + i * 2);
      }
      *(c16*)((uptr)address_ + (bit_offset_ - 1) * 2) = 0;
      break;

    case kTypeCheatCode:
      ((core::CheatCode*)address_)->Toggle();
      ((core::CheatCode*)address_)->Execute();
      break;

    default:
      break;
  }
}

void PageItem::EditNumber(const c16* text) {
  s64 integer;
  f64 decimal;
  if (is_read_only_ || !ParseNumber(text, integer, decimal)) return;

  switch (type_) {
    case kTypeU8:
    case kTypeAbility:
      StoreClamped<u8>(address_, integer, min_, max_, is_min_used_,
                       is_max_used_);
      break;
    case kTypeS8:
      StoreClamped<s8>(address_, integer, min_, max_, is_min_used_,
                       is_max_used_);
      break;
    case kTypeU16:
    case kTypeSpecies:
    case kTypeMove:
    case kTypeItem:
      StoreClamped<u16>(address_, integer, min_, max_, is_min_used_,
                        is_max_used_);
      break;
    case kTypeS16:
      StoreClamped<s16>(address_, integer, min_, max_, is_min_used_,
                        is_max_used_);
      break;
    case kTypeU32:
    case kTypePointer:
      StoreClamped<u32>(address_, integer, min_, max_, is_min_used_,
                        is_max_used_);
      break;
    case kTypeS32:
      StoreClamped<s32>(address_, integer, min_, max_, is_min_used_,
                        is_max_used_);
      break;
    case kTypeU64:
      *(u64*)address_ = (u64)integer;
      break;
    case kTypeS64:
      *(s64*)address_ = integer;
      break;
    case kTypeBits: {
      s64 value = integer;
      if (is_min_used_ && value < min_) value = min_;
      if (is_max_used_ && value > max_) value = max_;
      SetBits((u32*)address_, bit_offset_, bit_size_, (u32)value);
      break;
    }
    case kTypeF32:
    case kTypeF64: {
      f64 value = decimal;
      if (is_min_used_ && value < min_) value = min_;
      if (is_max_used_ && value > max_) value = max_;
      if (type_ == kTypeF32) {
        *(f32*)address_ = (f32)value;
      } else {
        *(f64*)address_ = value;
      }
      break;
    }
    case kTypeBoolean:
      *(bool*)address_ = integer != 0;
      break;
    default:
      break;
  }
}

void PageItem::GetNumberText(c16* buffer) const {
  switch (type_) {
    case kTypeF32:
      core::Utils::Format(buffer, u"%.3f", *(f32*)address_);
      TrimZeros(buffer);
      break;
    case kTypeF64:
      core::Utils::Format(buffer, u"%.3f", *(f64*)address_);
      TrimZeros(buffer);
      break;
    case kTypeU32:
    case kTypePointer:
      core::Utils::Format(buffer, u"%u", *(u32*)address_);
      break;
    case kTypeU64:
      core::Utils::Format(buffer, u"%llu", *(u64*)address_);
      break;
    case kTypeS64:
      core::Utils::Format(buffer, u"%lld", *(s64*)address_);
      break;
    default:
      core::Utils::Format(buffer, u"%d", GetIndex());
      break;
  }
}

void PageItem::Execute(MainApplication& application) {
  if (kTypeMenu == type_) {
    application.Open((menu_callback_t)address_, args_, name_);
  } else if (callback_ != nullptr) {
    callback_(args_);
  }
}
} // namespace ui
