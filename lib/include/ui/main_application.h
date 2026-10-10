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
 * @file main_application.h
 * @brief The menu of the plugin.
 *
 * @see docs/concepts/menu.md
 * @see docs/tutorials/02-add-a-menu-page.md
 */

#pragma once

#include <type_traits>

#include "core/cheat_code_manager.h"
#include "system/native/controller.h"
#include "ui/application.h"
#include "ui/page_item.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/widget/keyboard.h"
#include "ui/widget/numpad.h"

namespace sys {
class Controller;
}

namespace ui {
struct Theme;
/**
 * @brief The menu of the plugin.
 *
 * The menu shows one page at a time. A page is a function that adds entries
 * with Add(). The With functions change the last entry.
 *
 * The controls: Up / Down select an entry (hold to go faster), L / R jump
 * to the previous / next section, Left / Right change the value (hold to go
 * faster), A runs the entry (or switches On / Off), Y pins the entry to the
 * quick access, X applies the numpad or the keyboard, B goes back. The
 * bottom screen shows the description of the entry and a touch editor for
 * its value. OnChange() writes the data back after each change.
 *
 * @code
 * void LoadMyPage(ui::MainApplication& app, void* args) {
 *   app.Add("Speed", speed).WithBounds(1, 4)
 *      .Add("Battle", ui::LoadBattlePage);
 * }
 * @endcode
 */
class MainApplication : public Application {
public:
  friend class Painter;
  friend class MainAppPainter;

  STATIC_INLINE MainApplication& GetInstance() { return instance_; }

  /// Draws the entries of the page on the top screen.
  void DrawTop(sys::Graphics& graphics) override;

  /// Draws the numpad or the keyboard on the bottom screen.
  void DrawBottom(sys::Graphics& graphics) override;

  /// Reads the buttons and updates the menu. Call it one time for each frame.
  void Update(sys::Controller& controller) override;

  /// Returns true when the menu is open.
  bool IsOpened() const { return is_opened_; };

  /// Closes the menu.
  void ForceClose();

  /**
   * @brief Opens a page over the current page (8 pages at most).
   * @param load_menu The page function.
   * @param args A value that the page function receives.
   * @param title The name of the page. The bottom screen shows it.
   */
  void Open(menu_callback_t load_menu, void* args = nullptr,
            const c8* title = nullptr);

  /// Closes the current page and goes back to the previous page.
  void Close();

  /// Sets the object that draws the menu.
  void SetPainter(Painter& painter) {
    painter_ = &painter;
  }

  /**
   * @brief Shows a text for each value of the last entry (0 = first text).
   * @param array The texts.
   * @param array_size The number of texts.
   */
  MainApplication& WithArray(const c8* array[], u32 array_size) {
    entries_[entries_count_ - 1].WithArray(array, array_size);
    return *this;
  }

  /// Runs a function when the player changes or selects the last entry.
  MainApplication& WithCallback(callback_t callback) {
    entries_[entries_count_ - 1].WithCallback(callback);
    return *this;
  }

  /// Builds the page again when the value of the last entry changes.
  MainApplication& WithRefresh() {
    entries_[entries_count_ - 1].WithRefresh();
    return *this;
  }

  /// Sets the minimum value of the last entry.
  MainApplication& WithMin(s32 min) {
    entries_[entries_count_ - 1].WithMin(min);
    return *this;
  }

  /// Sets the maximum value of the last entry.
  MainApplication& WithMax(s32 max) {
    entries_[entries_count_ - 1].WithMax(max);
    return *this;
  }

  /// Sets the minimum and the maximum value of the last entry.
  MainApplication& WithBounds(u32 min, u32 max) {
    entries_[entries_count_ - 1].WithMin(min);
    entries_[entries_count_ - 1].WithMax(max);
    return *this;
  }

  /// Sets the step of a decimal value.
  MainApplication& WithFactor(f32 factor) {
    entries_[entries_count_ - 1].WithFactor(factor);
    return *this;
  }

  /// Sets the text that the bottom screen shows for the last entry. Use
  /// one or two short sentences.
  MainApplication& WithDescription(const c8* description) {
    if (entries_count_ > 0) {
      entries_[entries_count_ - 1].WithDescription(description);
    }
    return *this;
  }

  /// The player can see the value of the last entry, but cannot change it.
  MainApplication& WithReadOnly() {
    if (entries_count_ > 0) entries_[entries_count_ - 1].WithReadOnly();
    return *this;
  }

  /// The menu shows the icons of the game for the value (see
  /// PageItem::WithIcons()).
  MainApplication& WithIcons(IconKind kind, const u16* ids = nullptr) {
    if (entries_count_ > 0) {
      entries_[entries_count_ - 1].WithIcons(kind, ids);
    }
    return *this;
  }

  /**
   * @brief Runs a function after each change of a value on this page.
   *
   * Use it when the entries change a copy of the game data: the function
   * writes the copy back at once (for example an encrypted Pokémon). The
   * player then never needs a "Save" entry.
   */
  MainApplication& OnChange(callback_t callback) {
    on_change_ = callback;
    return *this;
  }

  /// Hides the background of the menu on this page.
  MainApplication& WithNoBackground() {
    no_background_ = 1;
    return *this;
  }

  /**
     * @brief Checks that the game is in a process (for example the overworld).
     * @param vtable The vtable of the process, for example overworld::address::kVtable.
     * @return true when the game is in a different process. The page must
     *         then return at once: the menu closes it.
     */
  bool CheckProcess(uptr vtable);

  /// Builds the current page again. The page stays open.
  void Refresh();

  /// Adds an entry that enables or disables a cheat code.
  MainApplication& Add(const c8* name, CheatCodeId id) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_].Initialize(
          name, core::CheatCodeManager::GetInstance().Get(id), kTypeCheatCode);
      entries_count_++;
    }
    return *this;
  }

  /// Adds an entry that runs a function when the player presses A.
  MainApplication& Add(const c8* name, callback_t callback = nullptr) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_].Initialize(name, nullptr, kTypeIdle);
      entries_[entries_count_].WithCallback(callback);
      entries_count_++;
    }
    return *this;
  }

  MainApplication& Add(const c8* name, void* addr, u8 type) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, type);
    return *this;
  }

  /// Adds an entry that opens a page.
  MainApplication& Add(const c8* name, menu_callback_t menu,
                       void* args = nullptr) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_].Initialize(name, (void*)menu, kTypeMenu);
      entries_[entries_count_].WithArgs(args);
      entries_count_++;
    }
    return *this;
  }

  MainApplication& Add(const c8* name, void*& addr) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, &addr, kTypePointer);
    return *this;
  }

  /// Adds an On/Off entry.
  MainApplication& Add(const c8* name, bool& addr) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, &addr, kTypeBoolean);
    return *this;
  }

  /// Adds an entry for some bits of a value.
  MainApplication& Add(const c8* name, void* addr, u32 offset, u32 size) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, kTypeBits, offset,
                                            size);
    return *this;
  }

  /**
   * @brief Adds an entry for a text (UTF-16).
   * @param name The name of the entry.
   * @param addr The address of the text.
   * @param size The maximum number of characters.
   * @return The menu, to add more entries.
   */
  MainApplication& Add(const c8* name, c16* addr, u32 size) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, kTypeUnicode, size);
    return *this;
  }

  /// Adds an entry that shows the name of a species.
  MainApplication& AddSpecies(const c8* name, u16& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeSpecies);
      WithBounds(0, static_cast<u32>(pokemon::SpeciesId::kCount)-1);
      WithIcons(IconKind::kPokemon);
    }
    return *this;
  }

  /// Adds an entry that shows the name of a type.
  MainApplication& AddType(const c8* name, u8& var) {
    static const c8* TYPES[] = {
        "Normal", "Fighting", "Flying", "Poison", "Ground", "Rock",
        "Bug", "Ghost", "Steel", "Fire", "Water", "Grass",
        "Electric", "Psychic", "Ice", "Dragon", "Dark", "Fairy"
    };
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeU8);
      WithArray(TYPES, SIZE(TYPES));
      WithIcons(IconKind::kType);
    }
    return *this;
  }

  /// Adds an entry that shows the name of an ability.
  MainApplication& AddAbility(const c8* name, u8& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeAbility);
      WithBounds(0, 0xBF);
    }
    return *this;
  }

  /// Adds an entry that shows the name of a move.
  MainApplication& AddMove(const c8* name, u16& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeMove);
      WithBounds(0, 0x26E);
    }
    return *this;
  }

  /// Adds an entry that shows the name of an item.
  MainApplication& AddItem(const c8* name, u16& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeItem);
      WithBounds(0, 775);
      WithIcons(IconKind::kItem);
    }
    return *this;
  }

  /// Adds an empty line. L and R jump from one separator to the next.
  MainApplication& AddSeparator() {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize("", nullptr, kTypeSeparator);
    }
    return *this;
  }

  /// Adds a section title. L and R jump from one section to the next.
  MainApplication& AddSection(const c8* name) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, nullptr, kTypeSeparator);
    }
    return *this;
  }

  /// Adds the pinned entries and the recent entries. The player pins an
  /// entry with Y. Use it at the start of the first page.
  MainApplication& AddQuickAccess();

#define ADD(type, type_id)                                      \
  MainApplication &Add(const c8 *name, type &var) {                  \
    if (entries_count_ < kMaxEntries)                           \
      entries_[entries_count_++].Initialize(name, (void *)&var, \
                                            kType##type_id);    \
    return *this;                                               \
  }

  ADD(u8, U8)
  ADD(u16, U16)
  ADD(u32, U32)
  ADD(u64, U64)
  ADD(s8, S8)
  ADD(s16, S16)
  ADD(s32, S32)
  ADD(s64, S64)
  ADD(f32, F32)
  ADD(f64, F64)

#undef ADD

  /// Adds an entry for an `enum class` value. The entry uses the integer type
  /// of the enum, with the same size.
  // The code uses `typename ...::type`: the `_t` aliases do not exist in
  // C++11.
  template <typename Enum,
            typename = typename std::enable_if<std::is_enum<Enum>::value>::type>
  MainApplication& Add(const c8* name, Enum& var) {
    return Add(
        name,
        reinterpret_cast<typename std::underlying_type<Enum>::type&>(var));
  }

  // The same method for the entries below. Different enums have the same size.
  // For example, `AddType` accepts `MoveType` and `TypeId`. One template for
  // each entry type replaces one function for each enum.
#define ADD_ENUM_ALIAS(method, raw)                                       \
  template <typename Enum, typename =                                     \
            typename std::enable_if<std::is_enum<Enum>::value>::type>     \
  MainApplication& method(const c8* name, Enum& var) {                    \
    static_assert(sizeof(Enum) == sizeof(raw),                            \
                  #method " requires an enum of the same width as " #raw);\
    return method(name, reinterpret_cast<raw&>(var));                     \
  }

  ADD_ENUM_ALIAS(AddSpecies, u16)
  ADD_ENUM_ALIAS(AddType, u8)
  ADD_ENUM_ALIAS(AddAbility, u8)
  ADD_ENUM_ALIAS(AddMove, u16)
  ADD_ENUM_ALIAS(AddItem, u16)

#undef ADD_ENUM_ALIAS

private:
  struct MenuContext {
    u8 cursor;
    u8 offset;
    u8 display_count;
    menu_callback_t load_menu;
    void* args;
    const c8* title; ///< The name of the entry that opened the page.

    MenuContext()
      : cursor(0),
        offset(0),
        display_count(0),
        load_menu(nullptr),
        args(nullptr),
        title(nullptr) {
    }

    void Initialize(menu_callback_t menu, void* args, const c8* title) {
      cursor = 0;
      offset = 0;
      display_count = 0;
      this->load_menu = menu;
      this->args = args;
      this->title = title;
    }
  };

  /// A page in the way to an entry: the page function, its value and its
  /// name.
  struct Frame {
    menu_callback_t load;
    void* args;
    const c8* title;
  };

  /// A pinned or a recent entry. The menu keeps the way to the entry as a
  /// text ("Battle > Settings > [Display] Type Helper"), saves it in the
  /// settings file, and finds the entry again when the first page loads.
  struct Shortcut {
    PageItem item; ///< The entry, after the menu found it.
    callback_t on_change; ///< The OnChange() function of its page.
    c8 path[128]; ///< The pages, [the section], the name of the entry.
    c8 name[32]; ///< A copy of the name of the entry.
    bool is_available; ///< false: the page needs a different process.
  };

  /// One result of the search.
  struct SearchResult {
    Frame frames[8]; ///< The pages from the first page (kMaxContexts).
    u8 depth; ///< The number of pages after the first page.
    c8 section[24];
    c8 name[32];
    c8 where[64]; ///< The names of the pages, for the description.
  };

  /// The last change, to cancel it.
  struct UndoState {
    PageItem item; ///< The entry that changed.
    u8 index; ///< Its index in the page.
    u8 size; ///< The size of the old value, in bytes.
    bool is_valid;
    u8 bytes[128]; ///< The old value.
  };

  /// The touch buttons of the bottom screen.
  enum TouchId : u8 {
    kTouchSectionUp, ///< The five buttons of the navigation bar.
    kTouchSectionDown,
    kTouchPin,
    kTouchSearch,
    kTouchBack,
    kTouchUndo, ///< At the top-right corner, after a change.
    kTouchOff, ///< The two buttons of an On / Off entry.
    kTouchOn,
    kTouchRun, ///< The button of a page or of an action.
    kTouchMinus10,
    kTouchMinus1,
    kTouchPlus1,
    kTouchPlus10,
    kTouchRadial0, ///< The 8 buttons of the radial menu of the pins.
    kTouchChoice0 = kTouchRadial0 + 8, ///< The kMaxChoices buttons of a list.
    /// The kPickerCells cells of the grid of icons.
    kTouchPicker0 = kTouchChoice0 + 12,
    kTouchPagePrevious = kTouchPicker0 + IconPool::kGridSlotCount, ///< Pages.
    kTouchPageNext,
    kTouchMax,
  };

  /// The editor of the bottom screen for the selected entry.
  enum class Editor : u8 {
    kNone,
    kRun, ///< A page or an action.
    kSwitch, ///< On / Off.
    kChoices, ///< A short list of texts.
    kNumber, ///< The numpad and the step buttons.
    kText, ///< The keyboard.
    kPicker, ///< The grid of icons (see PageItem::WithIcons()).
  };

  MainApplication()
    : is_opened_(0),
      entries_count_(0),
      contexts_count_(0),
      no_background_(0),
      is_quick_access_(0),
      is_indexing_(0),
      index_failed_(0),
      is_search_page_(0),
      are_settings_loaded_(0),
      is_radial_open_(0),
      is_radial_held_(0),
      process_vtable_(0),
      entries_(page_entries_),
      theme_(Theme::GetInstance()) {
    InitializeTouch();
  }

  MenuContext& GetContext() {
    return contexts_[contexts_count_ > 0 ? contexts_count_ - 1 : 0];
  }

  PageItem& GetSelectedEntry() {
    MenuContext& ctx = GetContext();
    return entries_[ctx.cursor + ctx.offset];
  }

  u32 GetSelectedIndex() {
    return GetContext().cursor + GetContext().offset;
  }

  bool AreKeysReleased(sys::Controller& ctrl);

  /// Selects the entry at `index` and scrolls to show it.
  void Select(u32 index);
  /// Moves the cursor by `delta` selectable entries.
  void MoveCursor(s32 delta, bool wrap);
  /// Moves the cursor to the next (1) or previous (-1) section.
  void JumpSection(s32 direction);
  /// Moves the cursor to a selectable entry after a page loads.
  void FinishLoad();
  /// Starts the animation of a new page: 1 for a deeper page, -1 back.
  void StartTransition(s32 direction);
  /// Moves the animations by one frame.
  void StepAnimations();

  // The quick access (menu_shortcuts.cc).

  /// Pins or unpins the entry at `index`.
  void TogglePin(u32 index);
  /// Returns the pin with this way, or -1.
  s32 FindPin(const c8* path) const;
  /// Adds the entry at `index` to the list of the recent entries.
  void AddRecent(u32 index);
  /// Adds the entries of a shortcut list to the page.
  void AddShortcuts(Shortcut* list, u32 count);
  /// Writes the way to the entry at `index`. Returns false when a page has
  /// no name.
  bool BuildPath(u32 index, c8* path, u32 size);
  /// Returns the shortcut at `index` of the quick access, or null.
  Shortcut* GetShortcut(u32 index);
  /// Removes a shortcut from a list.
  static void Remove(Shortcut* list, u8& count, u32 index);
  /// Reads the section (kSectionLength) and the name (kNameLength) of the
  /// entry at the end of a way.
  static void SplitPath(const c8* path, c8* section, c8* name);

  // The pages without the screen (menu_shortcuts.cc).

  /// Runs a page function into a second list of entries. The current page
  /// does not change. Returns false when the page needs a different process.
  bool LoadScratch(menu_callback_t load, void* args);
  /// Finds the entry of a shortcut way. Fills the pages of the way.
  bool Resolve(const c8* path, Frame* frames, u32& depth, PageItem* item,
               callback_t* on_change);
  /// Opens the pages of a way, then selects the entry.
  void NavigateTo(const Frame* frames, u32 depth, const c8* section,
                  const c8* name);
  /// Opens the page of a shortcut and selects its entry.
  void OpenShortcut(const Shortcut& shortcut);

  // The search (menu_shortcuts.cc).

  /// The page of the search: the text, then the results.
  static void LoadSearchPage(MainApplication& app, void* args);
  /// Looks for the text of the search in all the pages.
  void RunSearch();
  /// Opens the page of a result.
  void OpenSearchResult(u32 index);

  // The radial menu of the pins (menu_shortcuts.cc).

  /// Reads the buttons while the radial menu is open.
  void UpdateRadial(sys::Controller& controller);
  /// Draws the radial menu on the bottom screen.
  void DrawRadial();

  // The settings file (menu_settings.cc).

  /// Reads sdmc:/sango/menu.ini: the theme, the options and the shortcuts.
  void LoadSettings();
  /// Writes sdmc:/sango/menu.ini when a setting changed.
  void SaveSettings();

  /// Keeps the old value of an entry before a change.
  void SaveUndo(const PageItem& entry, u32 index);
  /// Puts the old value back. A second undo puts the new value back.
  void Undo();
  /// Runs the OnChange() function of the entry at `index`, when there is one.
  void RunOnChange(u32 index);
  /// Shows the value of the selected entry in the numpad or the keyboard.
  void PrepareEditor(const PageItem& entry);
  /// Returns the editor of the bottom screen for an entry.
  Editor GetEditor(const PageItem& entry) const;
  /// Places the touch buttons of the bottom screen.
  void InitializeTouch();
  /// The actions that the buttons and the touch screen ask for in one frame.
  struct Input {
    s32 move = 0; ///< The number of entries to move (Up / Down).
    s32 section = 0; ///< -1: previous section, 1: next section.
    s32 step = 0; ///< The change of the value (Left / Right, -10 / +10).
    s32 choice = -1; ///< The index that the player touched, or -1.
    bool back = false;
    bool pin = false;
    bool run = false;
    bool apply = false; ///< Applies the numpad or the keyboard.
    bool search = false;
    bool undo = false;
  };

  /// Reads the touch buttons of the bottom screen into `input`.
  void ReadTouch(const PageItem& entry, Input& input);
  /// Draws one touch button.
  void DrawTouch(TouchId id, const c16* label, bool is_active) const;
  /**
   * @brief Draws the grid of icons of an entry (Editor::kPicker): the page
   *        that contains the value, and the page buttons.
   * @param value The current value of the entry.
   */
  void DrawPicker(const PageItem& entry, s32 value) const;
  /// Draws a text, with a shadow when the theme asks for it.
  void DrawLabel(s32 x, s32 y, const c16* text, Color color) const;

  static constexpr u32 kMaxEntries = 64;
  static constexpr u32 kMaxContexts = 8;
  static constexpr u32 kMaxDisplayCount = 15;
  static constexpr u32 kLineHeight = 16;
  static constexpr u32 kMaxPins = 8;
  static constexpr u32 kMaxRecents = 4;
  static constexpr u32 kMaxChoices = 12;
  /// The cells of the grid of icons: 6 columns, 3 rows.
  static constexpr u32 kPickerColumns = 6;
  static constexpr u32 kPickerCells = IconPool::kGridSlotCount;
  static constexpr u32 kMaxResults = 16;
  static constexpr u32 kQueryLength = 24;
  static constexpr u32 kSectionLength = 24;
  static constexpr u32 kNameLength = 32;
  /// After this number of frames, Up and Down move faster.
  static constexpr u32 kFastScrollFrames = 45;
  /// The number of frames of the flash after a change.
  static constexpr u32 kFlashFrames = 12;

  static MainApplication instance_;

  // The counters can reach kMaxEntries (64) and kMaxContexts (8):
  // 7 bits and 4 bits.
  u32 is_opened_ : 1;
  u32 entries_count_ : 7;
  u32 contexts_count_ : 4;
  u32 no_background_ : 1;
  u32 is_quick_access_ : 1; ///< The page shows the quick access.
  u32 is_indexing_ : 1; ///< The page loads into the second list.
  u32 index_failed_ : 1; ///< CheckProcess failed while indexing.
  u32 is_search_page_ : 1; ///< The page is the search page.
  u32 are_settings_loaded_ : 1;
  u32 is_radial_open_ : 1;
  u32 is_radial_held_ : 1; ///< X stays pressed since it opened the menu.
  u32  : 12;

  uptr process_vtable_;

  PageItem page_entries_[kMaxEntries]; ///< The entries of the page.
  PageItem scratch_entries_[kMaxEntries]; ///< See LoadScratch().
  PageItem* entries_; ///< page_entries_, or scratch_entries_ while indexing.
  u8 scratch_count_ = 0;
  callback_t scratch_on_change_; ///< The OnChange() of the scratch page.

  MenuContext contexts_[kMaxContexts];
  Numpad numpad_;
  Keyboard keyboard_;
  Button touch_[kTouchMax];
  Editor editor_ = Editor::kNone; ///< The editor of the last frame.
  u32 hold_frames_ = 0; ///< The frames that Up or Down stays pressed.
  u32 hold_step_frames_ = 0; ///< The frames that Left or Right stays pressed.
  callback_t on_change_; ///< The OnChange() function of the page.
  u8 prepared_index_ = 0xFF; ///< The entry that the editor shows.
  bool needs_prepare_ = true; ///< The editor must show the value again.

  Shortcut pins_[kMaxPins];
  Shortcut recents_[kMaxRecents];
  u8 pin_count_ = 0;
  u8 recent_count_ = 0;
  u8 touch_index_ = 0; ///< The selected entry when the buttons were reset.
  u8 first_pin_ = 0xFF; ///< The index of the first pin in the page.
  u8 first_recent_ = 0xFF; ///< The index of the first recent entry.
  s8 radial_direction_ = -1; ///< The selected pin of the radial menu.

  c16 search_query_[kQueryLength] = {}; ///< The text of the search.
  SearchResult results_[kMaxResults];
  u8 result_count_ = 0;

  UndoState undo_;

  // The animations. The values go to 0 (or to the target) frame by frame.
  f32 cursor_y_ = -1; ///< The y of the selection bar on the top screen.
  f32 transition_ = 0; ///< 1 when a page opens, then 0.
  s8 transition_direction_ = 1; ///< 1: deeper page, -1: back.
  u8 flash_index_ = 0xFF; ///< The entry that changed.
  u8 flash_frames_ = 0;

  Theme& theme_;
  Painter* painter_ = nullptr;
};
} // namespace ui
