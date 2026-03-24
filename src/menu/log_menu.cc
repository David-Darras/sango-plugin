#include "menu/log_menu.h"

#include <cstring>

#include "system/graphics.h"
#include "utils.h"

namespace menu {

LogMenu LogMenu::instance_ = LogMenu();

LogMenu::LogMenu() : is_enabled_(false) {
  for (u32 i = 0; i < kMaxEntries; i++) {
    log_entries_[i][0] = u'\0';
  }
}

void LogMenu::Draw() {
  if (!is_enabled_) return;

  Graphics::FillScreen(0, 0, 1, 0.6f);

  Color text_color{1, 1, 1, 1};

  for (u32 i = 0; i < kMaxEntries; i++) {
    if (log_entries_[i][0] == u'\0') continue;

    int x = 5;
    int y = 5 + i * kLineHeight;

    Graphics::DrawText(x, y, log_entries_[i], text_color);
  }
}

void LogMenu::Add(const c16* message, ...) {
  if (!message) return;

  c16 buffer[BUFFER_SIZE];

  va_list args;
  va_start(args, message);
  ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
      buffer, BUFFER_SIZE, message, args);
  va_end(args);

  for (u32 i = 0; i < kMaxEntries - 1; i++) {
    std::memcpy(log_entries_[i], log_entries_[i + 1],
                sizeof(c16) * kMaxEntryLength);
  }

  std::memcpy(log_entries_[kMaxEntries - 1], buffer,
              sizeof(c16) * kMaxEntryLength);
}

}  // namespace menu