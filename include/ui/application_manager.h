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

#ifndef SANGO_PLUGIN_APPLICATION_MANAGER_H
#define SANGO_PLUGIN_APPLICATION_MANAGER_H
#include "application.h"
#include "common.h"

namespace ui {
class ApplicationManager {
  SINGLETON(ApplicationManager)

public:
  static ApplicationManager& GetInstance() {
    static ApplicationManager instance;
    return instance;
  }

  void Push(Application& application) {
    if (application_ != nullptr) {
      application.SetParent(application_);
    }

    application_ = &application;
  }

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

#endif //SANGO_PLUGIN_APPLICATION_MANAGER_H