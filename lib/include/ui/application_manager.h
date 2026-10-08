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
 * @file application_manager.h
 * @brief The stack of the applications of the plugin.
 */

#pragma once
#include "ui/application.h"
#include "common.h"

namespace ui {
/// The applications of the plugin. The last one receives the buttons and draws.
class ApplicationManager {
  MAKE_SINGLETON(ApplicationManager)
public:
  /// Shows an application over the current one.
  void Push(Application& application) {
    if (application_ != nullptr) {
      application.SetParent(application_);
    }

    application_ = &application;
  }

  /// Closes the current application and goes back to the previous one.
  void Pop() {
    if (application_ != nullptr) {
      Application* parent = application_->GetParent();
      application_ = parent;
    }
  }

  Application* GetCurrentApplication() const {
    return application_;
  }

private:
  Application* application_ = nullptr;
};
}