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

#include "core/native/game_event.h"
#include "core/native/game_manager.h"
#include "core/utils.h"

namespace core {

class EventManager {
  SINGLETON(EventManager)

public:
  STATIC_INLINE EventManager& GetInstance() {
    return GameManager::GetInstance().GetGameEventManager();
  }

  GameEvent* GetGameEvent() const { return current_game_event_; }

  const char* GetCurrentEventName(uptr& vtable) const {
    if (current_game_event_ == nullptr || current_game_event_->vtable ==
        nullptr)
      return "";
    vtable = (uptr)current_game_event_->vtable;
    return core::Utils::GetClassNameFromVTable(current_game_event_->vtable);
  }

  INLINE void Patch(void (*on_load)(uptr), void (*on_update)(uptr)) {
    if (current_game_event_ == nullptr) return;
    uptr vtable = (uptr)current_game_event_->vtable;
    if (current_game_event_->sequence == 0) {
      on_load(vtable);
    } else {
      on_update(vtable);
    }
  }

  GameEvent* current_game_event_;
  GameManager* game_manager_;
  GameEvent* pending_game_event_;
  bool executed_;
};
}
