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
 * @file page_system.cc
 * @brief The menu pages of the System family.
 */

#include "ui/log_application.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/performance_overlay.h"

namespace ui {
namespace {
// The lines of the log as UTF-8: the names of the entries of the log page.
constexpr u32 kLogLineLength = 96;
c8 log_lines[LogApplication::GetLineCount()][kLogLineLength];

// Converts a UTF-16 text into UTF-8.
void ToUtf8(const c16* text, c8* out, u32 size) {
  u32 length = 0;
  for (; *text != 0; text++) {
    const c16 c = *text;
    if (c < 0x80) {
      if (length + 2 > size) break;
      out[length++] = (c8)c;
    } else if (c < 0x800) {
      if (length + 3 > size) break;
      out[length++] = (c8)(0xC0 | (c >> 6));
      out[length++] = (c8)(0x80 | (c & 0x3F));
    } else {
      if (length + 4 > size) break;
      out[length++] = (c8)(0xE0 | (c >> 12));
      out[length++] = (c8)(0x80 | ((c >> 6) & 0x3F));
      out[length++] = (c8)(0x80 | (c & 0x3F));
    }
  }
  out[length] = '\0';
}
} // namespace

void LoadLogPage(MainApplication& app, void* args) {
  app.Add("Clear The Log", [](void*) {
       LogApplication::Clear();
       MainApplication::GetInstance().Refresh();
     })
     .Add("Read Again", [](void*) { MainApplication::GetInstance().Refresh(); })
     .WithDescription("Shows the new messages of the log.")
     .AddSection("Messages (the newest last)");
  u32 count = 0;
  for (u32 i = 0; i < LogApplication::GetLineCount(); i++) {
    const c16* line = LogApplication::GetLine(i);
    if (line[0] == 0) continue;
    ToUtf8(line, log_lines[i], kLogLineLength);
    // The bottom screen shows the full line.
    app.Add(log_lines[i]).WithDescription(log_lines[i]);
    count++;
  }
  if (count == 0) app.Add("The log is empty.");
}

void LoadSystemPage(MainApplication& app, void* args) {
  app.AddSection("Sound");
  LoadSoundPage(app, args);
  app.AddSection("Game Time");
  LoadGameTimePage(app, args);
  app.AddSection("Developer")
     .Add("Performance Overlay",
          PerformanceOverlay::GetInstance().is_enabled)
     .WithDescription("Shows the frames per second and the free memory at "
                      "the bottom of the top screen, when the menu is closed.")
     .Add("Log", LoadLogPage)
     .WithDescription("The last messages of the plugin. L + R also shows "
                      "them.");
}
} // namespace ui
