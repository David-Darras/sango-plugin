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

#ifndef SANGO_PLUGIN_GAME_EVENT_MANAGER_H
#define SANGO_PLUGIN_GAME_EVENT_MANAGER_H

#include "game_manager.h"

class GameManager;
class GameEvent;

/**
 * @brief Represents an individual game event or scene logic.
 * This struct maps to the engine's internal event structure used for
 * sequenced gameplay logic.
 */
class GameEvent {
  void* vtable_;       ///< Pointer to the virtual method table.
  GameEvent* parent_;  ///< Pointer to the parent event, if any.
  u32 sequence_;       ///< Current sequence identifier within the event.
  void* heap_;         ///< Pointer to the heap memory allocated for this event.
  u32 state_;          ///< Current execution state of the event.
  void* ro_;           ///< Pointer to the associated executable module.
};

/**
 * @brief Singleton manager that coordinates game-wide events and transitions.
 */
class GameEventManager {
  /** @brief Private constructor to prevent manual instantiation. */
  GameEventManager() = default;

 public:
  /**
   * @brief Retrieves the singleton instance of the GameEventManager.
   * @return A reference to the active event manager instance.
   */
  static GameEventManager& GetInstance() {
    return GameManager::GetInstance().GetGameEventManager();
  }

  /**
   * @brief Gets the currently active game event.
   * @return A reference to the active GameEvent.
   */
  GameEvent& GetGameEvent() const { return *current_game_event_; }

  // Member fields mapped to the engine memory layout
  GameEvent* current_game_event_;  ///< The current event being processed.
  GameManager* game_manager_;      ///< Pointer back to the parent GameManager.
  GameEvent* pending_game_event_;  ///< The event scheduled to be loaded next.
  bool executed_;  ///< Flag indicating if event processing is active.
};

#endif  // SANGO_PLUGIN_GAME_EVENT_MANAGER_H