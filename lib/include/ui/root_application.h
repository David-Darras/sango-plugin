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

#ifndef SANGO_PLUGIN_ROOT_APPLICATION_H
#define SANGO_PLUGIN_ROOT_APPLICATION_H
#include "application.h"
#include "log_application.h"
#include "main_application.h"
#include "system/device.h"

namespace ui {
class RootApplication : public Application {
  MAKE_SINGLETON(RootApplication)
public:
  void DrawTop(Graphics& graphics) override {
    switch (choice) {
      case AppChoice::kMainApp:
        main_app_.DrawTop(graphics);
        break;
      case AppChoice::kLogApp:
        log_app_.DrawTop(graphics);
        break;
    }
  }

  void DrawBottom(Graphics& graphics) override {
    switch (choice) {
      case AppChoice::kMainApp:
        main_app_.DrawBottom(graphics);
        break;
      case AppChoice::kLogApp:
        log_app_.DrawBottom(graphics);
        break;
    }
  }

  void Update(Controller& controller) override {
    old_state = current_state;
    current_state = controller.IsKeyDown(Key::kR) && controller.
                    IsKeyDown(Key::kL);

    if (old_state && !current_state) {
      switch (choice) {
        case AppChoice::kMainApp:
          choice = AppChoice::kLogApp;
          break;
        case AppChoice::kLogApp:
          choice = AppChoice::kMainApp;
          break;
      }
    }

    switch (choice) {
      case AppChoice::kMainApp:
        main_app_.Update(controller);
        break;
      case AppChoice::kLogApp:
        log_app_.Update(controller);
        break;
    }
  }

private:
  enum class AppChoice : u32 {
    kClose,
    kMainApp,
    kLogApp
  };

  AppChoice choice = AppChoice::kMainApp;
  bool old_state = false;
  bool current_state = false;

  MainApplication& main_app_ = MainApplication::GetInstance();
  LogApplication& log_app_ = LogApplication::GetInstance();
};
} // namespace ui

#endif //SANGO_PLUGIN_ROOT_APPLICATION_H