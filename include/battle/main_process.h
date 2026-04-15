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

#ifndef SANGO_PLUGIN_BATTLE_MAIN_PROCESS_H
#define SANGO_PLUGIN_BATTLE_MAIN_PROCESS_H

#include "core/core.h"

class GameProcessManager;

namespace battle {

struct Config;

class MainProcess {
 public:
  static MainProcess& GetInstance() {
    return *(MainProcess*)(ADDRESS_BATTLE_MAIN_PROCESS);
  }

 private:
  void* vtable_;
  u32 _0[6];

  GameProcessManager* game_process_manager_;
  void* heaps_[6];

  Config* config_;
  void* main_data_;

  u32 _1[2];
};

}  // namespace battle

#endif  // SANGO_PLUGIN_BATTLE_MAIN_PROCESS_H
