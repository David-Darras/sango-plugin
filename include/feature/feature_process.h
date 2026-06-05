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

#ifndef SANGO_PLUGIN_FEATURE_PROCESS_H
#define SANGO_PLUGIN_FEATURE_PROCESS_H
#include "feature_app.h"
#include "game/process_manager.h"

namespace feature {
class ProcessHookContext {
  MAKE_SINGLETON(ProcessHookContext)
public:
  enum State {
    kIdle,
    kEnterOverworld,
    kOverworld,
  };

  const char* current_process = nullptr;
  State state = kIdle;

  static void OnEnterOverworld() {
    AppHookContext::OnEnterOverworld();
  }

  static void DoEachFrame() {
    auto& ctx = GetInstance();

    ctx.current_process = game::ProcessManager::GetInstance().
        GetCurrentProcessName();

    if (strcmp(ctx.current_process, PROCESS_NAME_FIELD_MAP) == 0) {
      if (ctx.state == kIdle) {
        ctx.state = kEnterOverworld;
      } else if (ctx.state == kEnterOverworld) {
        OnEnterOverworld();
        ctx.state = kOverworld;
      } else if (ctx.state == kOverworld) {
        ctx.state = kOverworld;
      }
    } else {
      ctx.state = kIdle;
    }
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_PROCESS_H