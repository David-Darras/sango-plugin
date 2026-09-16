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

#include "script/native/loaded_message.h"

namespace script {

struct SharedWorkspaces {
  LoadedMessage* loaded_message;
  LoadedMessage* swapped_message;
  void* yes_no_menu;
  void* list_menu;
  void* talk_message;
  void* nickname_input;
  void* message_window;
  void* event_models;
};

} // namespace script
