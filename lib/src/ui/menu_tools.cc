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
 * @file menu_tools.cc
 * @brief The tools of the entries of the menu: the messages, the menu of an
 *        entry (copy, paste, reset, pin, undo) and the help of the controls.
 *
 * A long press on Y (or on the description) opens the menu of the selected
 * entry. The menu shows six buttons in place of the editor of the value.
 *
 * The declarations are in ui/main_application.h.
 */

#include <cstring>

#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "ui/main_application.h"
#include "ui/menu_layout.h"

namespace ui {
using namespace layout;

namespace {
// The labels of the buttons of the menu of an entry (ContextAction).
const c16* const kContextLabels[] = {u"Copy", u"Paste", u"Reset",
                                     u"Pin", u"Undo", u"Help"};

// The columns of the buttons of the menu of an entry (see the buttons of a
// list in MainApplication::InitializeTouch()).
constexpr u32 kContextColumns = 3;

// Copies a text and cuts it to `capacity` characters.
void CopyText(c16* out, const c16* text, u32 capacity) {
  u32 i = 0;
  for (; i + 1 < capacity && text[i] != 0; i++) out[i] = text[i];
  out[i] = 0;
}
} // namespace

// The messages.

void MainApplication::ShowToast(const c8* message) {
  c16 text[256];
  core::Utils::Format(text, u"%s", message);
  ShowToastText(text);
}

void MainApplication::ShowToastText(const c16* message) {
  CopyText(toast_, message, kToastLength);
  toast_frames_ = kToastFrames;
}

// The values.

void MainApplication::GetValueTextOf(const PageItem& entry, u64 value,
                                     c16* buffer) {
  if (!entry.CanWriteValue()) {
    entry.GetValueText(buffer);
    return;
  }
  // A copy of the entry reads a copy of the data: the other bits of a
  // kTypeBits value stay.
  u64 storage = 0;
  memcpy(&storage, entry.GetAddress(), entry.GetValueSize());
  PageItem copy = entry;
  copy.SetAddress(&storage);
  copy.WriteValue(value);
  copy.GetValueText(buffer);
}

void MainApplication::CopyValue(u32 index) {
  if (index >= entries_count_) return;
  const PageItem& entry = entries_[index];
  if (entry.GetType() == kTypeUnicode && entry.HasValue()) {
    // Paste writes all the characters of the entry: the end stays empty.
    memset(clip_text_, 0, sizeof(clip_text_));
    CopyText(clip_text_, (const c16*)entry.GetAddress(),
             entry.GetTextCapacity() + 1 < SIZE(clip_text_)
                 ? entry.GetTextCapacity() + 1
                 : SIZE(clip_text_));
    is_clip_text_ = true;
    is_clip_integer_ = false;
  } else if (entry.IsInteger()) {
    clip_number_ = entry.ReadNumber();
    is_clip_text_ = false;
    is_clip_integer_ = true;
  } else if (entry.IsDecimal() && entry.HasValue()) {
    clip_raw_ = entry.ReadValue();
    is_clip_text_ = false;
    is_clip_integer_ = false;
  } else {
    theme_.Play(theme_.error_sound);
    ShowToast("This entry has no value to copy.");
    return;
  }
  has_clip_ = true;
  clip_type_ = entry.GetType();
  c16 value[256];
  entry.GetValueText(value);
  CopyText(clip_label_, value, SIZE(clip_label_));
  theme_.Play(theme_.confirm_sound);
  c16 message[256];
  core::Utils::Format(message, u"Copied: %ls", clip_label_);
  ShowToastText(message);
}

bool MainApplication::CanPaste(const PageItem& entry) const {
  if (!has_clip_ || !entry.HasValue() || entry.IsReadOnly()) return false;
  if (is_clip_text_) return entry.GetType() == kTypeUnicode;
  if (is_clip_integer_) return entry.IsInteger();
  // A decimal number needs the same type (f32 or f64).
  return entry.IsDecimal() && entry.GetType() == clip_type_;
}

void MainApplication::PasteValue(u32 index) {
  if (index >= entries_count_) return;
  PageItem& entry = entries_[index];
  if (!CanPaste(entry)) {
    theme_.Play(theme_.error_sound);
    ShowToast(has_clip_ ? "The copied value does not fit this entry."
                        : "Copy a value first: hold Y on an entry.");
    return;
  }
  if (is_clip_text_) {
    // A copy: the page can load again.
    const PageItem selected = entry;
    SaveUndo(entry, index);
    entry.Edit(clip_text_);
    theme_.Play(theme_.confirm_sound);
    FinishChange(index, selected);
  } else if (is_clip_integer_) {
    // The value goes into the limits of the entry.
    s64 value = clip_number_;
    s64 min;
    s64 max;
    if (entry.GetRange(min, max)) {
      if (value < min) value = min;
      if (value > max) value = max;
    }
    ApplyValue(index, (u64)value, false);
  } else {
    ApplyValue(index, clip_raw_, false);
  }
  c16 message[256];
  core::Utils::Format(message, u"Pasted: %ls", clip_label_);
  ShowToastText(message);
}

// The menu of an entry.

void MainApplication::OpenContext() {
  if (entries_count_ == 0 || !painter_->ShowBottom()) return;
  theme_.Play(theme_.next_sound);
  is_context_open_ = true;
  context_cursor_ = kContextCopy;
  for (u32 i = 0; i < kTouchMax; i++) touch_[i].Reset();
}

bool MainApplication::IsContextActionEnabled(u32 action) {
  const u32 index = GetSelectedIndex();
  const PageItem& entry = entries_[index];
  switch (action) {
    case kContextCopy:
      return entry.IsInteger() ||
             (entry.HasValue() && (entry.IsDecimal() ||
                                   entry.GetType() == kTypeUnicode));
    case kContextPaste:
      return CanPaste(entry);
    case kContextReset:
      return IsChanged(index) && entry.CanWriteValue();
    case kContextPin:
      return entry.IsSelectable();
    case kContextUndo:
      return undo_count_ != 0;
    default:
      return true;
  }
}

void MainApplication::RunContextAction(u32 action) {
  const u32 index = GetSelectedIndex();
  is_context_open_ = false;
  for (u32 i = 0; i < kTouchMax; i++) touch_[i].Reset();
  switch (action) {
    case kContextCopy:
      CopyValue(index);
      break;
    case kContextPaste:
      PasteValue(index);
      break;
    case kContextReset:
      ResetEntry(index);
      break;
    case kContextPin:
      TogglePin(index);
      break;
    case kContextUndo:
      Undo();
      break;
    case kContextHelp:
      theme_.Play(theme_.next_sound);
      is_help_open_ = true;
      break;
    default:
      break;
  }
}

void MainApplication::UpdateContext(sys::Controller& controller) {
  s32 selected = -1;
  for (u32 i = 0; i < kContextCount; i++) {
    touch_[kTouchChoice0 + i].Update();
    if (touch_[kTouchChoice0 + i].IsReleased()) selected = i;
  }

  // The +Control Pad moves in the grid of the buttons.
  s32 cursor = context_cursor_;
  if (controller.IsKeyRepeated(Key::kRight)) cursor++;
  if (controller.IsKeyRepeated(Key::kLeft)) cursor--;
  if (controller.IsKeyRepeated(Key::kDown)) cursor += kContextColumns;
  if (controller.IsKeyRepeated(Key::kUp)) cursor -= kContextColumns;
  if (cursor < 0) cursor += kContextCount;
  if (cursor >= (s32)kContextCount) cursor -= kContextCount;
  if (cursor != context_cursor_) {
    theme_.Play(theme_.next_sound);
    context_cursor_ = (u8)cursor;
  }

  if (controller.IsKeyReleased(Key::kA)) selected = context_cursor_;
  if (controller.IsKeyReleased(Key::kB)) {
    theme_.Play(theme_.close_sound);
    is_context_open_ = false;
    return;
  }
  if (selected < 0) return;
  if (!IsContextActionEnabled(selected)) {
    theme_.Play(theme_.error_sound);
    return;
  }
  RunContextAction(selected);
}

void MainApplication::DrawContext(const PageItem& entry) {
  const u32 index = GetSelectedIndex();
  c8 path[sizeof(Shortcut::path)];
  bool is_pinned = false;
  const Shortcut* shortcut = GetShortcut(index);
  if (shortcut != nullptr) {
    is_pinned = FindPin(shortcut->path) >= 0;
  } else if (BuildPath(index, path, sizeof(path))) {
    is_pinned = FindPin(path) >= 0;
  }

  for (u32 i = 0; i < kContextCount; i++) {
    const c16* label = kContextLabels[i];
    if (i == kContextPin && is_pinned) label = u"Unpin";
    DrawTouch((TouchId)(kTouchChoice0 + i), label, context_cursor_ == i,
              IsContextActionEnabled(i));
  }

  c16 text[128];
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  if (has_clip_) {
    core::Utils::Format(text, u"Copied value: %ls", clip_label_);
  } else {
    core::Utils::Format(text, u"No copied value yet.");
  }
  DrawLabel(16, kEditorY + 72, text, theme_.unselected_text_color);
  DrawLabel(16, kEditorY + 86,
            u"+Control Pad and A, or touch a button. B: close.",
            theme_.unselected_text_color);
}

// The help.

void MainApplication::DrawHelp(Editor editor) {
  static const c16* const kLines[] = {
      u"Up / Down: select an entry. L / R: previous / next section.",
      u"Left / Right: change the value. Hold: faster.",
      u"A: open, run or switch. B: back.",
      u"Y: pin the entry. Hold Y: copy, paste, reset...",
      u"X: the pinned entries in a circle (push a direction).",
      u"L + R: the log of the plugin.",
      u"Undo (top right): cancel the last changes.",
      u"Reset: the value of the page opening. Hold it to see it.",
      u"A mark on the left of a line: the value changed.",
  };
  const c16* editor_line = u"";
  switch (editor) {
    case Editor::kNumber:
      editor_line = u"Numpad: type, then OK (X). Drag the bar: slider.";
      break;
    case Editor::kPicker:
      editor_line = u"Grid: touch a value, then lift the stylus.";
      break;
    case Editor::kChoices:
      editor_line = u"Touch a value, or press Left / Right.";
      break;
    case Editor::kText:
      editor_line = u"Type the text, then touch OK (or press X).";
      break;
    case Editor::kSwitch:
      editor_line = u"Press A or Left / Right, or touch On / Off.";
      break;
    case Editor::kRun:
      editor_line = u"Some actions run only after you hold A one second.";
      break;
    default:
      break;
  }

  sys::Graphics::SetTextScale(0.6f, 0.6f);
  DrawLabel(6, 6, u"Controls of the menu", theme_.selected_text_color);
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  s32 y = 28;
  for (const c16* line : kLines) {
    DrawLabel(6, y, line, theme_.unselected_text_color);
    y += 15;
  }
  if (editor_line[0] != 0) {
    DrawLabel(6, y + 4, editor_line, theme_.selected_text_color);
  }
  DrawLabel(6, 222, u"Touch the screen or press B to close.",
            theme_.unselected_text_color);
}
} // namespace ui
