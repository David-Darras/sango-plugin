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

#include "ui/top_menu.h"
#include "core/engine.h"
#include "hack/hook_manager.h"
#include "menu/plugin_menu.h"
#include "renderer/light_manager.h"
#include "system/device.h"
#include "system/file.h"
#include "system/graphics.h"
#include "ui/application_manager.h"
#include "ui/menu_context.h"

void Initialize() {
  File::MountSdmc();

  HookManager& hook_manager = HookManager::GetInstance();
  Device::SetupHooks(hook_manager);
  core::Engine::SetupHooks(hook_manager);
  renderer::LightManager::SetupHooks(hook_manager);

  auto& application_manager = ui::ApplicationManager::GetInstance();
  auto& plugin_menu = menu::PluginMenu::GetInstance();
  auto& menu_context = ui::TopMenuContext::GetInstance();

  plugin_menu.Open(ui::LoadTopMenu, &menu_context);
  application_manager.Push(plugin_menu);
}

void Entrypoint() {
  auto& graphics = Graphics::GetInstance();
  auto& controller = Controller::GetInstance();
  auto& application_manager = ui::ApplicationManager::GetInstance();
  auto* application = application_manager.GetCurrentApplication();

  application->Update(controller);

  void* top_buffer = graphics.GetFramebuffer(Screen::kTop);
  if (graphics.BindFramebuffer(top_buffer)) {
    Graphics::EnableScissor(0, 0, 400, 240);
    Graphics::BeginRender(top_buffer);
    application->DrawTop(graphics);
    Graphics::DisableScissor();
  }

  void* bottom_buffer = graphics.GetFramebuffer(Screen::kBottom);
  if (graphics.BindFramebuffer(bottom_buffer)) {
    Graphics::EnableScissor(0, 0, 320, 240);
    Graphics::BeginRender(bottom_buffer);
    application->DrawBottom(graphics);
    Graphics::DisableScissor();
  }
}