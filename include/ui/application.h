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

#ifndef SANGO_PLUGIN_APPLICATION_H
#define SANGO_PLUGIN_APPLICATION_H

#include "common.h"

class Controller;
class Graphics;

namespace ui {
class Application {
public:
  virtual ~Application() = default;
  virtual void Update(Controller& controller) = 0;
  virtual void DrawTop(Graphics& graphics) = 0;
  virtual void DrawBottom(Graphics& graphics) = 0;

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

#endif //SANGO_PLUGIN_APPLICATION_H