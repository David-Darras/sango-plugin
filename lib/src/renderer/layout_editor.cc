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
 * @file layout_editor.cc
 * @brief Finds the panes of the 2D layouts of the screen, and keeps the
 *        changes that the player makes to them.
 *
 * The declarations are in renderer/patch/layout_editor.h.
 */

#include "renderer/patch/layout_editor.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "core/hook.h"
#include "core/ini.h"
#include "core/native/process_manager.h"
#include "core/utils.h"
#include "renderer/address.h"
#include "renderer/native/picture.h"
#include "renderer/native/text_box.h"
#include "system/native/file.h"
#include "system/native/graphics.h"

namespace renderer {

namespace {
core::Hook<void(Pane*, const void*)> update_matrix_hook;

const c16* const kPath = u"sdmc:/sango/layouts.ini";

// The flag byte of a pane: the bit 0 is the visibility, and the bits 4 and 5
// are the state of its matrix: 0 means "calculate it again".
constexpr u8 kVisibleBit = 0x01;
constexpr u8 kMatrixConditionMask = 0x30;

// The deepest tree and the largest list of children that the editor reads.
constexpr u32 kMaxDepth = 24;
constexpr u32 kMaxChildren = 512;

u32 Hash(const c8* text) {
  u32 hash = 2166136261u;
  for (; *text != '\0'; text++) {
    hash = (hash ^ static_cast<u8>(*text)) * 16777619u;
  }
  return hash;
}

// A pointer to an object of the game: aligned, in the heap or in the linear
// memory. A pane that the game destroyed can leave a wrong pointer.
bool IsPointer(const void* pointer) {
  const uptr address = reinterpret_cast<uptr>(pointer);
  return (address & 3) == 0 && sys::Graphics::IsDataAddress(address);
}

// The children of a pane are a list of nodes. A node is the field `next_pane`
// of the child (the 4 bytes after its vtable), and the end of the list is the
// root node, in the parent.
bool IsEndOfList(const Pane* parent, uptr node) {
  return node == reinterpret_cast<uptr>(&parent->children.next_pane);
}

Pane* FromNode(uptr node) { return reinterpret_cast<Pane*>(node - 4); }

// The first child, or null.
Pane* FirstChild(const Pane* parent) {
  const uptr node = reinterpret_cast<uptr>(parent->children.next_pane);
  if (IsEndOfList(parent, node) || !IsPointer(FromNode(node))) return nullptr;
  return FromNode(node);
}

// The next child, or null at the end of the list.
Pane* NextChild(const Pane* parent, const Pane* child) {
  const uptr node = reinterpret_cast<uptr>(child->next_pane);
  if (IsEndOfList(parent, node) || !IsPointer(FromNode(node))) return nullptr;
  return FromNode(node);
}

bool Contains(const Pane* root, const Pane* wanted, u32 depth) {
  if (root == wanted) return true;
  if (depth >= kMaxDepth) return false;
  u32 count = 0;
  for (const Pane* child = FirstChild(root); child != nullptr;
       child = NextChild(root, child)) {
    if (++count > kMaxChildren) break;
    if (Contains(child, wanted, depth + 1)) return true;
  }
  return false;
}

u32 CountTree(const Pane* pane, u32 depth) {
  u32 total = 1;
  if (depth >= kMaxDepth) return total;
  u32 count = 0;
  for (const Pane* child = FirstChild(pane); child != nullptr;
       child = NextChild(pane, child)) {
    if (++count > kMaxChildren) break;
    total += CountTree(child, depth + 1);
  }
  return total;
}

// The text of a text box: the pointer to its buffer is in the struct. The size
// of the buffer in bytes, the length of the text in characters and a flag byte
// (the bit 0 tells the box to build its text again) come after the colors.
constexpr u32 kTextBufferBytesOffset = 0xF8;
constexpr u32 kTextLengthOffset = 0xFA;
constexpr u32 kTextFlagsOffset = 0xFE;
constexpr u8 kTextChangedBit = 0x01;

template <typename T>
T* AtOffset(const Pane* pane, u32 offset) {
  return reinterpret_cast<T*>(reinterpret_cast<uptr>(pane) + offset);
}

c16* TextBuffer(const Pane* pane) {
  c16* buffer = reinterpret_cast<const TextBox*>(pane)->text;
  const uptr address = reinterpret_cast<uptr>(buffer);
  return (address & 1) == 0 && sys::Graphics::IsDataAddress(address) ? buffer
                                                                      : nullptr;
}

// Writes a UTF-16 text as UTF-8. The backslash and the new line have an
// escape, so that the text fits in one line of the file.
void EncodeText(const c16* text, c8* out, u32 size) {
  u32 length = 0;
  for (; *text != 0; text++) {
    const c16 c = *text;
    if (length + 5 > size) break;
    if (c == '\\') {
      out[length++] = '\\';
      out[length++] = '\\';
    } else if (c == '\n') {
      out[length++] = '\\';
      out[length++] = 'n';
    } else if (c < 0x20) {
      continue;
    } else if (c < 0x80) {
      out[length++] = static_cast<c8>(c);
    } else if (c < 0x800) {
      out[length++] = static_cast<c8>(0xC0 | (c >> 6));
      out[length++] = static_cast<c8>(0x80 | (c & 0x3F));
    } else {
      out[length++] = static_cast<c8>(0xE0 | (c >> 12));
      out[length++] = static_cast<c8>(0x80 | ((c >> 6) & 0x3F));
      out[length++] = static_cast<c8>(0x80 | (c & 0x3F));
    }
  }
  out[length] = '\0';
}

void DecodeText(const c8* text, c16* out, u32 max) {
  u32 length = 0;
  while (*text != '\0' && length < max) {
    const u8 c = static_cast<u8>(*text++);
    if (c == '\\') {
      const c8 next = *text;
      if (next == 'n') {
        out[length++] = '\n';
        text++;
      } else if (next == '\\') {
        out[length++] = '\\';
        text++;
      }
    } else if (c < 0x80) {
      out[length++] = c;
    } else if ((c & 0xE0) == 0xC0 && text[0] != '\0') {
      out[length++] = static_cast<c16>(((c & 0x1F) << 6) | (text[0] & 0x3F));
      text += 1;
    } else if ((c & 0xF0) == 0xE0 && text[0] != '\0' && text[1] != '\0') {
      out[length++] = static_cast<c16>(((c & 0x0F) << 12) |
                                       ((text[0] & 0x3F) << 6) | (text[1] & 0x3F));
      text += 2;
    }
  }
  out[length] = 0;
}

// The colors of a pane: 2 for a text box, 4 for a picture.
Color8* GetColors(Pane* pane, u32& count) {
  switch (LayoutEditor::GetKind(pane)) {
    case PaneKind::kTextBox:
      count = 2;
      return &reinterpret_cast<TextBox*>(pane)->top_color;
    case PaneKind::kPicture:
      count = 4;
      return &reinterpret_cast<Picture*>(pane)->top_left_color;
    default:
      count = 0;
      return nullptr;
  }
}

// The vtables of the kinds of panes. The object points 8 bytes after
// the symbol of the vtable (the offset and the type information come first).
struct KindEntry {
  uptr vtable;
  PaneKind kind;
};
}  // namespace

void LayoutEditor::Initialize() {
#ifdef GAME_ORAS
  update_matrix_hook.Install(address::kPaneUpdateMatrix, UpdateMatrixHook);
  Load();
#endif
}

void LayoutEditor::Update() {
  auto& self = GetInstance();
  self.frame_++;
  for (Root& root : self.roots_) {
    if (root.pane == nullptr) continue;
    // A hidden root can leave the game's update: the editor keeps it, and it
    // shows it again when the mode ends. The pane can be gone: check it.
    if (root.is_hidden) {
      const bool is_valid = IsPointer(root.pane) &&
                            LayoutEditor::GetKind(root.pane) != PaneKind::kUnknown;
      if (!is_valid) {
        root.pane = nullptr;
        root.is_hidden = false;
        continue;
      }
      if (self.hide_all_) {
        root.pane->flags &= ~kVisibleBit;
        root.frame = self.frame_;
      } else {
        root.pane->flags |= kVisibleBit;
        root.is_hidden = false;
      }
      continue;
    }
    if (root.frame + 1 < self.frame_) root.pane = nullptr;
  }

  uptr vtable = 0;
  const c8* name = core::Utils::Unmangle(
      core::ProcessManager::GetInstance().GetCurrentProcessName(vtable));
  strncpy(self.screen_, name, sizeof(self.screen_) - 1);
  self.screen_[sizeof(self.screen_) - 1] = '\0';
  self.screen_hash_ = Hash(self.screen_);
}

const c8* LayoutEditor::GetScreenName() { return GetInstance().screen_; }

PaneKind LayoutEditor::GetKind(const Pane* pane) {
  static const KindEntry kKinds[] = {
      {address::kPaneVtable, PaneKind::kPane},
      {address::kTextBoxVtable, PaneKind::kTextBox},
      {address::kPictureVtable, PaneKind::kPicture},
      {address::kWindowVtable, PaneKind::kWindow},
      {address::kBoundingVtable, PaneKind::kBounding},
  };
  const uptr vtable = reinterpret_cast<uptr>(pane->vtable);
  for (const KindEntry& entry : kKinds) {
    if (entry.vtable != 0 &&
        (vtable == entry.vtable + 8 || vtable == entry.vtable)) {
      return entry.kind;
    }
  }
  return PaneKind::kUnknown;
}

LayoutEditor::Root* LayoutEditor::Register(Pane* pane) {
  Root* free_slot = nullptr;
  for (Root& root : roots_) {
    if (root.pane == pane) {
      root.frame = frame_;
      return &root;
    }
    if (root.pane == nullptr && free_slot == nullptr) free_slot = &root;
  }
  if (free_slot == nullptr) {
    // The oldest root leaves.
    free_slot = &roots_[0];
    for (Root& root : roots_) {
      if (root.frame < free_slot->frame) free_slot = &root;
    }
  }
  free_slot->pane = pane;
  free_slot->frame = frame_;
  // The size is the one of the game: Apply() comes after this call.
  free_slot->tag = static_cast<u32>(pane->scale2.x + 0.5f);
  free_slot->is_hidden = false;
  return free_slot;
}

void LayoutEditor::SetHideAll(bool is_hidden) {
  GetInstance().hide_all_ = is_hidden;
}

u32 LayoutEditor::GetRoots(Pane** out, u32 max) {
  auto& self = GetInstance();
  u32 count = 0;
  for (const Root& root : self.roots_) {
    if (root.pane == nullptr || !IsPointer(root.pane) || count >= max) continue;
    out[count++] = root.pane;
  }
  return count;
}

bool LayoutEditor::IsAlive(const Pane* pane) {
  if (pane == nullptr || !IsPointer(pane)) return false;
  Pane* roots[kMaxRoots];
  const u32 count = GetRoots(roots, kMaxRoots);
  for (u32 i = 0; i < count; i++) {
    if (Contains(roots[i], pane, 0)) return true;
  }
  return false;
}

u32 LayoutEditor::CountPanes(const Pane* pane) { return CountTree(pane, 0); }

Pane* LayoutEditor::GetChild(const Pane* pane, u32 index) {
  u32 i = 0;
  for (Pane* child = FirstChild(pane); child != nullptr;
       child = NextChild(pane, child)) {
    if (i++ == index) return child;
    if (i > kMaxChildren) break;
  }
  return nullptr;
}

u32 LayoutEditor::GetChildCount(const Pane* pane) {
  u32 count = 0;
  for (const Pane* child = FirstChild(pane); child != nullptr;
       child = NextChild(pane, child)) {
    if (++count >= kMaxChildren) break;
  }
  return count;
}

void LayoutEditor::GetPath(const Pane* pane, c8* out, u32 size) {
  // The names from the pane to the root, then in the right order.
  const Pane* chain[kMaxDepth];
  u32 depth = 0;
  for (const Pane* p = pane; p != nullptr && IsPointer(p) && depth < kMaxDepth;
       p = p->parent) {
    chain[depth++] = p;
  }
  out[0] = '\0';
  for (u32 i = depth; i > 0; i--) {
    const u32 length = strlen(out);
    const Pane* current = chain[i - 1];
    const c8* name = current->id;
    if (length + strlen(name) + 8 > size) break;
    if (length != 0) {
      out[length] = '/';
      out[length + 1] = '\0';
    }
    strcat(out, name);
    // The root of a layout: its name has the width of the layout, "@400".
    // The two screens can have panes with the same names and the same paths.
    if (current->parent == nullptr) {
      u32 tag = 0;
      for (const Root& root : GetInstance().roots_) {
        if (root.pane == current) tag = root.tag;
      }
      snprintf(out + strlen(out), 8, "@%lu", (unsigned long)tag);
    }
  }
}

PaneState LayoutEditor::Read(const Pane* pane) {
  PaneState state;
  state.translate[0] = pane->position.x;
  state.translate[1] = pane->position.y;
  state.translate[2] = pane->position.z;
  state.rotate[0] = pane->rotation.x;
  state.rotate[1] = pane->rotation.y;
  state.rotate[2] = pane->rotation.z;
  state.scale[0] = pane->scale.x;
  state.scale[1] = pane->scale.y;
  state.size[0] = pane->scale2.x;
  state.size[1] = pane->scale2.y;
  state.alpha = pane->alpha;
  state.is_visible = (pane->flags & kVisibleBit) != 0;
  u32 count = 0;
  const Color8* colors = GetColors(const_cast<Pane*>(pane), count);
  state.color_count = static_cast<u8>(count);
  for (u32 i = 0; i < count; i++) state.colors[i] = colors[i];
  return state;
}

void LayoutEditor::Write(Pane* pane, const PaneState& state) {
  pane->position.x = state.translate[0];
  pane->position.y = state.translate[1];
  pane->position.z = state.translate[2];
  pane->rotation.x = state.rotate[0];
  pane->rotation.y = state.rotate[1];
  pane->rotation.z = state.rotate[2];
  pane->scale.x = state.scale[0];
  pane->scale.y = state.scale[1];
  pane->scale2.x = state.size[0];
  pane->scale2.y = state.size[1];
  pane->alpha = state.alpha;
  pane->flags = state.is_visible ? (pane->flags | kVisibleBit)
                                 : (pane->flags & ~kVisibleBit);
  // The game keeps the matrix of a rotated pane until a flag says that it is
  // dirty: this tells it to calculate the matrix again.
  pane->flags &= ~kMatrixConditionMask;
  u32 count = 0;
  Color8* colors = GetColors(pane, count);
  for (u32 i = 0; i < count && i < state.color_count; i++) {
    colors[i] = state.colors[i];
  }
}

LayoutEditor::Edit* LayoutEditor::Find(const Pane* pane) {
  c8 path[kMaxPathLength];
  GetPath(pane, path, sizeof(path));
  for (Edit& edit : edits_) {
    if (edit.is_used && edit.screen_hash == screen_hash_ &&
        strcmp(edit.path, path) == 0) {
      return &edit;
    }
  }
  return nullptr;
}

u32 LayoutEditor::GetTextCapacity(const Pane* pane) {
  if (pane == nullptr || GetKind(pane) != PaneKind::kTextBox) return 0;
  if (TextBuffer(pane) == nullptr) return 0;
  const u32 bytes = *AtOffset<u16>(pane, kTextBufferBytesOffset);
  return bytes >= 4 ? bytes / 2 - 1 : 0;
}

void LayoutEditor::ReadText(const Pane* pane, c16* out) {
  out[0] = 0;
  const u32 capacity = GetTextCapacity(pane);
  if (capacity == 0) return;
  const c16* buffer = TextBuffer(pane);
  u32 length = 0;
  while (length < capacity && length < kMaxTextLength && buffer[length] != 0) {
    out[length] = buffer[length];
    length++;
  }
  out[length] = 0;
}

void LayoutEditor::WriteText(Pane* pane, const c16* text) {
  const u32 capacity = GetTextCapacity(pane);
  if (capacity == 0) return;
  c16* buffer = TextBuffer(pane);
  u32 length = 0;
  while (length < capacity && text[length] != 0) length++;
  // The same text: the box keeps its layout.
  bool is_same = buffer[length] == 0;
  for (u32 i = 0; is_same && i < length; i++) is_same = buffer[i] == text[i];
  if (is_same) return;
  for (u32 i = 0; i < length; i++) buffer[i] = text[i];
  buffer[length] = 0;
  *AtOffset<u16>(pane, kTextLengthOffset) = static_cast<u16>(length);
  *AtOffset<u8>(pane, kTextFlagsOffset) |= kTextChangedBit;
}

bool LayoutEditor::SetText(const Pane* pane, const c16* original,
                           const c16* text) {
  auto& self = GetInstance();
  if (self.Find(pane) == nullptr) {
    const PaneState state = Read(pane);
    if (!SetEdit(pane, state, state)) return false;
  }
  Edit* edit = self.Find(pane);
  if (edit == nullptr) return false;
  if (!edit->has_text) {
    edit->has_text = true;
    for (u32 i = 0; i <= kMaxTextLength; i++) {
      edit->original_text[i] = original[i];
      if (original[i] == 0) break;
    }
    edit->original_text[kMaxTextLength] = 0;
  }
  for (u32 i = 0; i <= kMaxTextLength; i++) {
    edit->text[i] = text[i];
    if (text[i] == 0) break;
  }
  edit->text[kMaxTextLength] = 0;
  return true;
}

const c16* LayoutEditor::GetTextEdit(const Pane* pane) {
  const Edit* edit = GetInstance().Find(pane);
  return edit != nullptr && edit->has_text ? edit->text : nullptr;
}

const c16* LayoutEditor::GetTextOriginal(const Pane* pane) {
  const Edit* edit = GetInstance().Find(pane);
  return edit != nullptr && edit->has_text ? edit->original_text : nullptr;
}

bool LayoutEditor::SetEdit(const Pane* pane, const PaneState& original,
                           const PaneState& state) {
  auto& self = GetInstance();
  Edit* edit = self.Find(pane);
  if (edit == nullptr) {
    for (Edit& slot : self.edits_) {
      if (!slot.is_used) {
        edit = &slot;
        break;
      }
    }
    if (edit == nullptr) return false;
    edit->is_used = true;
    edit->screen_hash = self.screen_hash_;
    strncpy(edit->screen, self.screen_, sizeof(edit->screen) - 1);
    edit->screen[sizeof(edit->screen) - 1] = '\0';
    GetPath(pane, edit->path, sizeof(edit->path));
    edit->original = original;
    edit->has_text = false;
    edit->text[0] = 0;
    edit->original_text[0] = 0;
    self.edit_count_++;
  }
  edit->state = state;
  return true;
}

const PaneState* LayoutEditor::GetEdit(const Pane* pane) {
  const Edit* edit = GetInstance().Find(pane);
  return edit != nullptr ? &edit->state : nullptr;
}

const PaneState* LayoutEditor::GetOriginal(const Pane* pane) {
  const Edit* edit = GetInstance().Find(pane);
  return edit != nullptr ? &edit->original : nullptr;
}

// Writes the original values to the panes of the screen that the change
// touches: the pane that the player chose, and any other pane with the same
// path.
namespace {
void RestoreTree(Pane* pane, const c8* path, const PaneState& original,
                 const c16* original_text, u32 depth) {
  if (depth >= kMaxDepth) return;
  c8 current[LayoutEditor::kMaxPathLength];
  LayoutEditor::GetPath(pane, current, sizeof(current));
  if (strcmp(current, path) == 0) {
    LayoutEditor::Write(pane, original);
    if (original_text != nullptr) LayoutEditor::WriteText(pane, original_text);
  }
  u32 count = 0;
  for (Pane* child = FirstChild(pane); child != nullptr;
       child = NextChild(pane, child)) {
    if (++count > kMaxChildren) break;
    RestoreTree(child, path, original, original_text, depth + 1);
  }
}
}  // namespace

void LayoutEditor::Restore(const Edit& edit) {
  Pane* roots[kMaxRoots];
  const u32 count = GetRoots(roots, kMaxRoots);
  for (u32 i = 0; i < count; i++) {
    RestoreTree(roots[i], edit.path, edit.original,
                edit.has_text ? edit.original_text : nullptr, 0);
  }
}

void LayoutEditor::RemoveEdit(Pane* pane) {
  auto& self = GetInstance();
  Edit* edit = self.Find(pane);
  if (edit == nullptr) return;
  // The panes of the other screens with the same path are not touched: the
  // path has the width of the layout.
  Restore(*edit);
  edit->is_used = false;
  self.edit_count_--;
}

void LayoutEditor::RemoveAllEdits() {
  auto& self = GetInstance();
  for (Edit& edit : self.edits_) {
    if (edit.is_used && edit.screen_hash == self.screen_hash_) {
      Restore(edit);
      edit.is_used = false;
      self.edit_count_--;
    }
  }
}

u32 LayoutEditor::GetEditCount() { return GetInstance().edit_count_; }

void LayoutEditor::UpdateMatrixHook(Pane* pane, const void* draw_info) {
  auto& self = GetInstance();
  if (pane != nullptr) {
    if (pane->parent == nullptr) {
      Root* root = self.Register(pane);
      if (self.hide_all_ && (pane->flags & kVisibleBit) != 0) {
        pane->flags &= ~kVisibleBit;
        root->is_hidden = true;
      }
    }
    if (self.edit_count_ != 0) self.Apply(pane);
  }
  update_matrix_hook(pane, draw_info);
}

void LayoutEditor::Apply(Pane* pane) {
  c8 path[kMaxPathLength];
  bool has_path = false;
  for (Edit& edit : edits_) {
    if (!edit.is_used || edit.screen_hash != screen_hash_) continue;
    // The last name of the path is the name of the pane: a cheap test first.
    const c8* name = strrchr(edit.path, '/');
    name = name != nullptr ? name + 1 : edit.path;
    const u32 name_length = strlen(pane->id);
    if (strncmp(name, pane->id, name_length) != 0 ||
        (name[name_length] != '\0' && name[name_length] != '@')) {
      continue;
    }
    if (!has_path) {
      GetPath(pane, path, sizeof(path));
      has_path = true;
    }
    if (strcmp(edit.path, path) == 0) {
      Write(pane, edit.state);
      if (edit.has_text) WriteText(pane, edit.text);
    }
  }
}

// The saved changes: one line for each, "screen|path|values".

namespace {
core::Ini ini;
}

bool LayoutEditor::Save() {
  auto& self = GetInstance();
  ini.Clear();
  ini.AddSection("layouts");
  ini.AddComment("screen|path of the pane|translate x y z, rotate x y z, "
                 "scale x y, size w h, alpha, visible, colors (RRGGBBAA)");
  u32 number = 0;
  for (const Edit& edit : self.edits_) {
    if (!edit.is_used) continue;
    const PaneState& s = edit.state;
    c8 key[16];
    c8 value[480];
    snprintf(key, sizeof(key), "edit%lu", (unsigned long)++number);
    int length = snprintf(
        value, sizeof(value),
        "%s|%s|%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%u,%u,%u",
        edit.screen, edit.path, (double)s.translate[0], (double)s.translate[1],
        (double)s.translate[2], (double)s.rotate[0], (double)s.rotate[1],
        (double)s.rotate[2], (double)s.scale[0], (double)s.scale[1],
        (double)s.size[0], (double)s.size[1], (unsigned)s.alpha,
        (unsigned)(s.is_visible ? 1 : 0), (unsigned)s.color_count);
    for (u32 i = 0; i < s.color_count && length > 0 &&
                    length + 10 < (int)sizeof(value);
         i++) {
      length += snprintf(value + length, sizeof(value) - length, ",%02X%02X%02X%02X",
                         s.colors[i].r, s.colors[i].g, s.colors[i].b,
                         s.colors[i].a);
    }
    if (edit.has_text && length > 0 && length + 4 < (int)sizeof(value)) {
      value[length++] = '|';
      EncodeText(edit.text, value + length, sizeof(value) - length);
    }
    ini.Set(key, value);
  }
  sys::File::CreateDirectory(u"sdmc:/sango");
  return ini.Save(kPath);
}

bool LayoutEditor::Load() {
  auto& self = GetInstance();
  if (!ini.Load(kPath)) return false;
  for (u32 number = 1; number <= kMaxEdits; number++) {
    c8 key[16];
    snprintf(key, sizeof(key), "edit%lu", (unsigned long)number);
    const c8* value = ini.Get("layouts", key);
    if (value == nullptr) continue;

    // "screen|path|values"
    c8 line[480];
    strncpy(line, value, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    c8* path = strchr(line, '|');
    if (path == nullptr) continue;
    *path++ = '\0';
    c8* values = strchr(path, '|');
    if (values == nullptr) continue;
    *values++ = '\0';
    // The text of a text box comes after the values.
    c8* text = strchr(values, '|');
    if (text != nullptr) *text++ = '\0';

    PaneState s;
    f32* numbers[10] = {&s.translate[0], &s.translate[1], &s.translate[2],
                        &s.rotate[0],    &s.rotate[1],    &s.rotate[2],
                        &s.scale[0],     &s.scale[1],     &s.size[0],
                        &s.size[1]};
    c8* cursor = values;
    bool is_valid = true;
    for (f32* number_out : numbers) {
      c8* end = nullptr;
      *number_out = strtof(cursor, &end);
      if (end == cursor || *end != ',') {
        is_valid = false;
        break;
      }
      cursor = end + 1;
    }
    if (!is_valid) continue;
    s.alpha = static_cast<u8>(strtoul(cursor, &cursor, 10));
    if (*cursor == ',') cursor++;
    s.is_visible = strtoul(cursor, &cursor, 10) != 0;
    if (*cursor == ',') cursor++;
    s.color_count = static_cast<u8>(strtoul(cursor, &cursor, 10));
    if (s.color_count > 4) s.color_count = 4;
    for (u32 i = 0; i < s.color_count && *cursor == ','; i++) {
      cursor++;
      const u32 rgba = strtoul(cursor, &cursor, 16);
      s.colors[i] = Color8(rgba >> 24, (rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF,
                           rgba & 0xFF);
    }

    Edit* slot = nullptr;
    for (Edit& edit : self.edits_) {
      if (!edit.is_used) {
        slot = &edit;
        break;
      }
    }
    if (slot == nullptr) break;
    slot->is_used = true;
    strncpy(slot->screen, line, sizeof(slot->screen) - 1);
    slot->screen[sizeof(slot->screen) - 1] = '\0';
    slot->screen_hash = Hash(slot->screen);
    strncpy(slot->path, path, sizeof(slot->path) - 1);
    slot->path[sizeof(slot->path) - 1] = '\0';
    // After a start the game values are not known: the first change of the
    // pane keeps the saved state as its original.
    slot->original = s;
    slot->state = s;
    slot->has_text = text != nullptr;
    slot->text[0] = 0;
    if (text != nullptr) DecodeText(text, slot->text, kMaxTextLength);
    for (u32 i = 0; i <= kMaxTextLength; i++) {
      slot->original_text[i] = slot->text[i];
      if (slot->text[i] == 0) break;
    }
    self.edit_count_++;
  }
  return true;
}

}  // namespace renderer
