#include "core/core.h"
#include "menu/plugin_menu.h"
#include "overworld/map_manager.h"

namespace overworld {

static u32 choice = 0;

void DoFieldMove(void*) {
  MapManager& map_manager = MapManager::GetInstance();

  struct {
    u16 zone_id;
    u16 team_index;
    MapManager* map_manager;
  } context = {map_manager.GetMapId(), 0, &map_manager};

  ((void (*)(void*, u32))ADDRESS_DO_FIELD_MOVE)(&context, choice);
}

void FieldMove_LoadMenu(menu::PluginMenu& menu, void* args) {
  static const c8* MOVES[] = {"Cut",         "Surf",       "Waterfall",
                              "Strength",    "Rock Smash", "Fly",
                              "Flash",       "Teleport",   "Dig",
                              "Sweet Scent", "Dive",       "Secret Power"};

  menu.Add("Choice", choice)
      .WithArray(MOVES, SIZE(MOVES))
      .Add("Execute", DoFieldMove);
}

}  // namespace overworld