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
 * @file page_layouts.cc
 * @brief The menu pages of the 2D layouts of the game (the panes of its
 *        menus).
 *
 * The first page lists the layouts that the screen draws. A layout is a tree of
 * panes: the second page shows one pane, with its parent, its children and its
 * brothers, and the player moves in the tree with these entries. The changes of
 * a pane stay (the game cannot undo them) and the player can save them on the
 * SD card (see renderer::LayoutEditor).
 */

#include <cstdio>
#include <cstring>

#include "renderer/patch/layout_editor.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using renderer::LayoutEditor;
using renderer::Pane;
using renderer::PaneKind;
using renderer::PaneState;

// The most entries of a list of children or of brothers. The menu has 64
// entries at most: the player reads the others with the "More" entry.
constexpr u32 kListSize = 12;
constexpr u32 kLabelLength = 44;

const c8* kKindNames[] = {"Group Of Panes", "Text Box", "Picture",
                          "Window", "Touch Area", "Unknown"};

// The pane of the page, and the values that its entries change. The values
// stay in the page while the pane is selected, so that the page of a color can
// change them too.
Pane* selected_pane = nullptr;
const void* opened_from = nullptr;
Pane* edited_pane = nullptr;
PaneState edit;
PaneState original;
bool has_edit = false;

u32 children_offset = 0;
u32 siblings_offset = 0;

c16 name_text[24];
c8 path_text[LayoutEditor::kMaxPathLength];
u8 kind_value = 0;
bool is_saved = false;
u32 edit_count = 0;

c8 root_labels[LayoutEditor::kMaxRoots][kLabelLength];
c8 child_labels[kListSize][kLabelLength];
c8 sibling_labels[kListSize][kLabelLength];

// The text of an entry that opens a pane: its name, its kind and the number
// of its children.
void MakeLabel(const Pane* pane, c8* out) {
  const u32 children = LayoutEditor::GetChildCount(pane);
  static const c8* const kShort[] = {"Group", "Text", "Picture", "Window",
                                     "Touch", "?"};
  const c8* kind = kShort[static_cast<u32>(LayoutEditor::GetKind(pane))];
  if (children != 0) {
    snprintf(out, kLabelLength, "%s  [%s]  +%lu", pane->id, kind,
             (unsigned long)children);
  } else {
    snprintf(out, kLabelLength, "%s  [%s]", pane->id, kind);
  }
}

// Goes to another pane of the tree. The page builds itself again.
void GoTo(Pane* pane) {
  selected_pane = pane;
  children_offset = 0;
  siblings_offset = 0;
  MainApplication::GetInstance().Refresh();
}

// A change of the page: the pane gets the values now, and at each frame.
void ApplyEdit(void*) {
  if (edited_pane == nullptr || !LayoutEditor::IsAlive(edited_pane)) return;
  has_edit = true;
  LayoutEditor::Write(edited_pane, edit);
  if (!LayoutEditor::SetEdit(edited_pane, original, edit)) {
    MainApplication::GetInstance().ShowToast(
        "24 changes at most: remove one first.");
  }
}

void ResetPane(void*) {
  if (edited_pane == nullptr || !LayoutEditor::IsAlive(edited_pane)) return;
  LayoutEditor::RemoveEdit(edited_pane);
  has_edit = false;
  edited_pane = nullptr;
  MainApplication::GetInstance().Refresh();
}

void SaveChanges(void*) {
  MainApplication::GetInstance().ShowToast(
      LayoutEditor::Save() ? "Saved to sdmc:/sango/layouts.ini."
                           : "The SD card did not save the file.");
}

void RemoveScreenChanges(void*) {
  LayoutEditor::RemoveAllEdits();
  has_edit = false;
  edited_pane = nullptr;
  MainApplication::GetInstance().Refresh();
}

void MoreChildren(void*) {
  children_offset += kListSize;
  if (children_offset >= LayoutEditor::GetChildCount(selected_pane)) {
    children_offset = 0;
  }
  MainApplication::GetInstance().Refresh();
}

void MoreSiblings(void*) {
  siblings_offset += kListSize;
  const Pane* parent = selected_pane->parent;
  if (parent == nullptr || siblings_offset >= LayoutEditor::GetChildCount(parent)) {
    siblings_offset = 0;
  }
  MainApplication::GetInstance().Refresh();
}
}  // namespace

void LoadLayoutsPage(MainApplication& app, void* args) {
#ifdef GAME_XY
  app.Add("The 2D layouts need the data of Alpha Sapphire.");
#else
  // A new visit: the page of a pane starts at its root again.
  opened_from = nullptr;
  edited_pane = nullptr;
  edit_count = LayoutEditor::GetEditCount();

  Pane* roots[LayoutEditor::kMaxRoots];
  const u32 count = LayoutEditor::GetRoots(roots, LayoutEditor::kMaxRoots);

  app.AddSection("Layouts On The Screen");
  if (count == 0) {
    app.Add("No 2D layout on the screen.")
       .WithDescription("Open a menu of the game, then look here again. The "
                        "plugin knows a layout when the game draws it.");
  }
  for (u32 i = 0; i < count; i++) {
    snprintf(root_labels[i], kLabelLength, "%s  (%lu panes)", roots[i]->id,
             (unsigned long)LayoutEditor::CountPanes(roots[i]));
    app.Add(root_labels[i], LoadPanePage, roots[i])
       .WithDescription("The panes of this layout, as a tree.");
  }

  app.AddSection("Changes")
     .Add("Saved Changes", edit_count)
     .WithReadOnly()
     .WithDescription("The changes of all the screens. They apply at each "
                      "frame, and after a start when they are saved.")
     .Add("Save To The SD Card", SaveChanges)
     .WithDescription("Writes the changes to sdmc:/sango/layouts.ini.")
     .Add("Remove The Changes Of This Screen", RemoveScreenChanges)
     .WithConfirm();
#endif
}

void LoadPanePage(MainApplication& app, void* args) {
  // The first time: the pane of the page of the layouts.
  if (args != nullptr && args != opened_from) {
    opened_from = args;
    selected_pane = static_cast<Pane*>(args);
    children_offset = 0;
    siblings_offset = 0;
  }
  if (selected_pane == nullptr || !LayoutEditor::IsAlive(selected_pane)) {
    app.Add("This pane is not on the screen any more.")
       .WithDescription("The game closed its layout. Go back and open the "
                        "layouts of the screen again.");
    return;
  }
  Pane* pane = selected_pane;

  // The values of the page: the saved change of the pane, or the values that
  // the game gives it now.
  if (pane != edited_pane) {
    edited_pane = pane;
    const PaneState* saved = LayoutEditor::GetEdit(pane);
    has_edit = saved != nullptr;
    if (has_edit) {
      edit = *saved;
      original = *LayoutEditor::GetOriginal(pane);
    } else {
      edit = LayoutEditor::Read(pane);
      original = edit;
    }
  } else if (has_edit) {
    // The page of a color changed the values: the pane gets them.
    ApplyEdit(nullptr);
  } else {
    edit = LayoutEditor::Read(pane);
    original = edit;
  }

  // The name, the kind and the path.
  u32 length = 0;
  for (; length + 1 < SIZE(name_text) && pane->id[length] != '\0'; length++) {
    name_text[length] = static_cast<c16>(static_cast<u8>(pane->id[length]));
  }
  name_text[length] = 0;
  LayoutEditor::GetPath(pane, path_text, sizeof(path_text));
  kind_value = static_cast<u8>(LayoutEditor::GetKind(pane));
  is_saved = has_edit;

  app.OnChange(ApplyEdit);
  app.AddSection("Pane")
     .Add("Name", name_text, SIZE(name_text))
     .WithReadOnly()
     .WithDescription(path_text)
     .Add("Kind", kind_value)
     .WithArray(kKindNames, SIZE(kKindNames))
     .WithReadOnly()
     .Add("Changed", is_saved)
     .WithReadOnly()
     .WithDescription("On: this pane has a change. The game cannot undo it.")

     .AddSection("Show")
     .Add("Visible", edit.is_visible)
     .Add("Alpha", edit.alpha)
     .WithDescription("The opacity: 0 is invisible, 255 is solid.")

     .AddSection("Move")
     .Add("Translate X", edit.translate[0])
     .Add("Translate Y", edit.translate[1])
     .Add("Translate Z", edit.translate[2])
     .Add("Rotate X", edit.rotate[0])
     .Add("Rotate Y", edit.rotate[1])
     .Add("Rotate Z", edit.rotate[2])
     .WithDescription("In degrees. Z turns the pane on the screen.")
     .Add("Scale X", edit.scale[0])
     .WithFactor(0.05f)
     .Add("Scale Y", edit.scale[1])
     .WithFactor(0.05f);

  // The size is the size of the image of a picture or of a window, and the
  // touch area of a button. A text box keeps its text in a buffer that the game
  // builds when the text changes, and a group draws nothing: for them, the size
  // changes nothing that the player can see. Scale does the same job.
  const PaneKind kind = LayoutEditor::GetKind(pane);
  const bool uses_size = kind == PaneKind::kPicture ||
                         kind == PaneKind::kWindow ||
                         kind == PaneKind::kBounding;
  app.Add("Width", edit.size[0])
     .Add("Height", edit.size[1]);
  if (!uses_size) {
    app.WithReadOnly()
       .WithDescription("This pane does not draw with its size. Use Scale.");
  } else if (kind == PaneKind::kBounding) {
    app.WithDescription("The size of the area that the stylus touches.");
  } else {
    app.WithDescription("The size of the image, before Scale.");
  }

  if (edit.color_count == 2) {
    app.AddSection("Colors")
       .Add("Top Color", LoadColor8Page, &edit.colors[0])
       .Add("Bottom Color", LoadColor8Page, &edit.colors[1]);
  } else if (edit.color_count == 4) {
    app.AddSection("Colors")
       .Add("Top Left Color", LoadColor8Page, &edit.colors[0])
       .Add("Top Right Color", LoadColor8Page, &edit.colors[1])
       .Add("Bottom Left Color", LoadColor8Page, &edit.colors[2])
       .Add("Bottom Right Color", LoadColor8Page, &edit.colors[3]);
  }

  app.AddSection("This Pane")
     .Add("Reset This Pane", ResetPane)
     .WithDescription("Removes the change of this pane.")
     .Add("Save All The Changes", SaveChanges)
     .WithDescription("Writes the changes to sdmc:/sango/layouts.ini.");

  // The tree: the parent, the children, then the brothers.
  Pane* parent = pane->parent;
  app.AddSection("Tree");
  if (parent != nullptr && LayoutEditor::IsAlive(parent)) {
    static c8 parent_label[kLabelLength];
    MakeLabel(parent, parent_label);
    app.Add(parent_label, [parent](void*) { GoTo(parent); })
       .WithDescription("The parent: the pane that holds this pane.");
  } else {
    app.Add("This is the root of the layout.");
  }

  const u32 child_count = LayoutEditor::GetChildCount(pane);
  if (child_count != 0) {
    app.AddSection("Children");
    for (u32 i = 0; i < kListSize; i++) {
      Pane* child = LayoutEditor::GetChild(pane, children_offset + i);
      if (child == nullptr) break;
      MakeLabel(child, child_labels[i]);
      app.Add(child_labels[i], [child](void*) { GoTo(child); });
    }
    if (child_count > kListSize) {
      app.Add("More Children", MoreChildren)
         .WithDescription("The list has more panes: this entry shows the next "
                          "ones.");
    }
  }

  if (parent != nullptr) {
    const u32 brother_count = LayoutEditor::GetChildCount(parent);
    if (brother_count > 1) {
      app.AddSection("Brothers");
      u32 shown = 0;
      u32 index = 0;
      for (u32 i = 0; shown < kListSize; i++) {
        Pane* brother = LayoutEditor::GetChild(parent, i);
        if (brother == nullptr) break;
        if (brother == pane) continue;
        if (index++ < siblings_offset) continue;
        MakeLabel(brother, sibling_labels[shown]);
        app.Add(sibling_labels[shown], [brother](void*) { GoTo(brother); });
        shown++;
      }
      if (brother_count - 1 > kListSize) {
        app.Add("More Brothers", MoreSiblings)
           .WithDescription("The list has more panes: this entry shows the "
                            "next ones.");
      }
    }
  }
}
}  // namespace ui
