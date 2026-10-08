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
 * @file application.h
 * @brief The base class of the applications of the plugin.
 */

#pragma once

#include "common.h"
#include "system/native/controller.h"

namespace sys {
class Controller;
class Graphics;
}

namespace ui {
/// An application of the plugin: something that reads the buttons and draws on the screens.
class Application {
public:
  virtual ~Application() = default;
  /// Reads the buttons. Called one time for each frame.
  virtual void Update(sys::Controller& controller) = 0;
  /// Draws on the top screen.
  virtual void DrawTop(sys::Graphics& graphics) = 0;
  /// Draws on the bottom screen.
  virtual void DrawBottom(sys::Graphics& graphics) = 0;

  INLINE void SetParent(Application* parent) {
    parent_ = parent;
  }

  INLINE Application* GetParent() const {
    return parent_;
  }

private:
  Application* parent_ = nullptr;
};
}
