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

#include <CTRPluginFramework/Menu/PluginMenu.hpp>
#include <CTRPluginFramework/System/Hook.hpp>

#include "entrypoint.h"
#include "core/core.h"

namespace CTRPluginFramework {
int main() {
  //==========================================
#ifdef ENABLE_SANGO_PLUGIN
  Initialize();
  CTRPluginFramework::Hook hook;
  hook.InitializeForMitm(ADDRESS_ENTRYPOINT, (uptr)Entrypoint);
  hook.Enable();
#endif // ENABLE_SANGO_PLUGIN
  //==========================================

  //==========================================
#ifdef ENABLE_DEFAULT_CTRPF
  PluginMenu menu;
  menu.SynchronizeWithFrame(true);
  menu.Run();
#endif // ENABLE_DEFAULT_CTRPF
  //==========================================

  return 0;
}
} // namespace CTRPluginFramework